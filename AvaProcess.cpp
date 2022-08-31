#include "AvaProcess.hpp"
#include "FrameworkManager.hpp"
#include "ModuleService.hpp"
#include "CommunicationManager.hpp"
#include "ModuleCommand.hpp"
#include "PicoIntent.hpp"
#include <zmq.hpp>
#include "AvaCommandResult.hpp"
#include "AvaModuleInteraction.hpp"

AvaProcess::AvaProcess(std::shared_ptr<IMicrophoneService> microphone, std::shared_ptr<zmq::context_t> context) :
	LoggerFactory{ this },
	framework_ { new FrameworkManager(microphone)},
	modules_{ new ModuleService() },
	comms_{ new CommunicationManager(context) },
	should_pause_ {false}
 {
	LoadModules();
}

std::unique_ptr<AvaCommandResult> AvaProcess::Run() {
	{
		std::unique_lock lock(control_mutex_);
		should_pause_ = true;
	}
	std::unique_lock lock(mutex_);
	{
		std::unique_lock lock(control_mutex_);
		should_pause_ = false;
	}
	framework_->SayText("What can I do for you sir?", true);
	std::unique_ptr<IIntent> intent{ framework_->GetIntent() };
	return RunSynchronized(move(intent));
}

std::unique_ptr<AvaCommandResult> AvaProcess::Run(ModuleRequest& module_req) {
	std::unique_lock lock(mutex_);
	std::unique_ptr<IIntent> intent = std::make_unique<PicoIntent>(module_req);
	return RunSynchronized(move(intent));
}

std::unique_ptr<AvaCommandResult> AvaProcess::RunSynchronized(std::unique_ptr<IIntent> intent) {

	if (intent->GetModule() == "AVA") {
		return ProcessAvaCommand(move(intent));
	}
	else {
		CommunicateModule(move(intent));
		std::unique_ptr<AvaCommandResult> r{ nullptr };
		return r;
	}

}

std::unique_ptr<AvaCommandResult> AvaProcess::ProcessAvaCommand(std::unique_ptr<IIntent> intent) {
	LogDebug() << "Got ava command. action: " << intent->GetAction();
	std::unique_ptr<AvaCommandResult> result = std::make_unique<AvaCommandResult>();
	if (intent->GetAction() == "turnoff") {
		result->SetOff(true);
	}
	else if (intent->GetAction() == "reload_modules") {
		framework_->SayText("Reloading modules!");
		ReloadModules();
		framework_->SayText("Modules have been reloaded");
	}
	else if (intent->GetAction() == "cancel_mod_comm") {
		result->SetCancelled(true);
	}
	intent.reset();
	return result;
}

void AvaProcess::CommunicateModule(std::unique_ptr<IIntent> intent) {
	LogDebug() << "Broadcasting intent: " << intent->ToString();
	zmq::message_t intent_msg(intent->ToString());
	comms_->PublishMsg(intent_msg);

	bool is_done = false;
	while (!is_done) {
		LogDebug() << "Waiting on modules response";
		zmq::message_t msg;
		comms_->RecvMsg(msg);
		
		{
			std::unique_lock lock(control_mutex_);
			if (should_pause_) {
				zmq::message_t done{ std::string{"pauseCmd:"}+msg.to_string() };
				if(!msg.empty()) comms_->SendMsg(done);
				should_pause_ = false;
				throw ModulePaused();
			}
		}

		if (msg.empty()) {
			LogWarn() << "Got not response from module.";
			is_done = true;
		}
		else {
			is_done = ProcessModuleMsg(msg);
		}

	}
}

bool AvaProcess::ProcessModuleMsg(zmq::message_t& msg) {
	std::string_view msg_view = msg.to_string_view();
	LogDebug() << "Got msg back. Msg: |" << msg_view << "|";
	if (msg_view == "done") {
		LogVerbose() << "No action required from ava framework. Continuing";
		zmq::message_t done{ std::string{"done"} };
		comms_->SendMsg(done);
		return true;
	}
	else {
		ModuleCommand mod_command(msg_view);
		if (mod_command.GetParam().size() > 50) {
			LogWarn() << "Got a param thats way too long.";
			zmq::message_t empty_msg{ std::string{"badRequest"} };
			comms_->SendMsg(empty_msg);
			return false;
		}
		LogDebug() << mod_command.ToString();
		if (mod_command.GetCommand() == "say") {

			framework_->SayText(mod_command.GetParam(), mod_command.IsAsync(), mod_command.GetLang());
			zmq::message_t empty_msg{ std::string{"okay"} };
			comms_->SendMsg(empty_msg);
		}
		else if (mod_command.GetCommand() == "say-listen") {

			framework_->SayText(mod_command.GetParam(), mod_command.IsAsync(), mod_command.GetLang());
			zmq::message_t dication(framework_->GetText(mod_command.GetTime()));
			LogDebug() << "Sending dictation: " << dication.str();
			comms_->SendMsg(dication);
		}
		else if (mod_command.GetCommand() == "confirm") {
			framework_->SayText(mod_command.GetParam(), mod_command.IsAsync(), mod_command.GetLang());
			std::unique_ptr<IIntent>  intent = framework_->GetConfirmation();
			if (intent == nullptr) {
				zmq::message_t confirmation(std::string{ "" });
				comms_->SendMsg(confirmation);
			}
			else {
				zmq::message_t confirmation(intent->GetAction());
				comms_->SendMsg(confirmation);
			}

		}
		return false;
	}
}

void AvaProcess::CancelComs(ModuleRequest& req) {

	std::unique_ptr<IIntent> intent = std::make_unique<PicoIntent>(req, false);

	zmq::message_t intent_msg(intent->ToString());
	LogInfo() << "Cancelling? " << intent_msg.to_string();
	comms_->PublishMsg(intent_msg);
}

void AvaProcess::SaySsml(std::string msg, bool async) {
	framework_->SaySsml(msg, async);
}

void AvaProcess::LoadModules() {
	modules_->LoadModules();
	comms_->WaitForSynModules(modules_->CountModules());
}

void AvaProcess::ReloadModules() {
	zmq::message_t stop{ std::string{"MODULES_stop"} };
	comms_->PublishMsg(stop);
	modules_->UnloadModules();
	LoadModules();
}

AvaProcess::~AvaProcess() {
	delete comms_;
	delete modules_;
	delete framework_;
}