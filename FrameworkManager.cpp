#include "FrameworkManager.hpp"
#include "IMicrophoneService.hpp"
#include "GoogleTextToSpeechService.hpp"
#include "GoogleSpeechToTextService.hpp"
#include "PicoSpeechToIntentService.hpp"
#include "PicoWakeUpService.hpp"
#include "PicoRecorderService.hpp"
#include "ModuleService.hpp"
#include "IIntent.hpp"
#include "ModuleRequest.hpp"
#include "ModuleCommunicationService.hpp"
#include "ModuleListenerService.hpp"
#include <zmq.hpp>
#include <thread>
#include <unordered_set>
#include <chrono>
#include <string_view>

using std::string;

namespace ConstStr {
	const string welcome =
		R"(<speak>
            Ava 1.0 <break time="200ms"/>
            <emphasis level="strong">ready for action!</emphasis>
        </speak>)";
}

FrameworkManager::FrameworkManager() : 
	FrameworkManager(std::make_shared<PicoRecorderService>()) {};
	
FrameworkManager::FrameworkManager(std::shared_ptr<IMicrophoneService> microphone_service) :
	LoggerFactory(this),
	text_to_speech_service_{ new GoogleTextToSpeechService() },
	wake_up_service_{ new PicoWakeUpService(microphone_service) },
	speech_to_intent_service_{ new PicoSpeechToIntentService(microphone_service) },
	speech_to_text_service_{ new GoogleSpeechToTextService(microphone_service) },
	zmq_context_{ new zmq::context_t(1) },
	zmq_pub_socket_{ new zmq::socket_t(*zmq_context_, ZMQ_PUB) },
	zmq_rep_socket_{ new zmq::socket_t(*zmq_context_, ZMQ_REP) },
	module_communication_{ new ModuleCommunicationService(*this, *zmq_context_) },
	communication_thread_{ &ModuleCommunicationService::Start, module_communication_ },
	module_listener_{ new ModuleListenerService(*zmq_context_, *module_communication_) },
	listener_thread_{ &ModuleListenerService::Start, module_listener_ },
	module_service_{ new ModuleService() }{

	try {
		LogInfo() << "Starting framework manager...";
		LogVerbose() << "Binding pub and rep sockets.";
		zmq_pub_socket_->bind("tcp://127.0.0.1:5500");
		zmq_rep_socket_->bind("tcp://127.0.0.1:5501");
		zmq_rep_socket_->set(zmq::sockopt::rcvtimeo, 5000);
		LogInfo() << "Starting modules...";

		LoadModules();
		
	}
	catch(zmq::error_t &t) {
		LogError() << t.what();
		exit(1);
	}

};

void FrameworkManager::LoadModules() {
	int subs = module_service_->CountModules();

	std::thread t1(&IModuleService::LoadModules, module_service_);

	std::unordered_set <string> list_of_intents;

	LogInfo() << "Connecting to modules...";
	std::string msg;
	while (subs > 0) {
		zmq::message_t msg;
		zmq::recv_result_t result = zmq_rep_socket_->recv(msg);
		if (msg.empty()) {
			LogError() << "Module connection timeout!";
			exit(1);
		}
		std::string content_msg = msg.to_string();

		if (content_msg.find(":") == -1 || content_msg.find("#") == -1) {
			LogError() << "Modules must have the following pattern. <name>:syn#<intent> message recv was " << content_msg;
			exit(1);
		}

		std::string_view module_name{ content_msg.data(), content_msg.find(":") };
		std::string_view syn_part{ content_msg.data() + content_msg.find(":") + 1,   content_msg.find("#") - content_msg.find(":") - 1 };
		std::string intent_name = content_msg.substr(content_msg.find("#") + 1, content_msg.size() - content_msg.find("#"));

		if (list_of_intents.find(intent_name) != list_of_intents.end()) {
			LogError() << "Found duplicate modules listening to the same intent category. Intent: " << intent_name;
			exit(1);
		}
		else if (syn_part != "syn" || module_name.size() == 0 || intent_name.size() == 0) {
			LogError() << "Modules must have the following pattern. <name>:syn#<intent> message recv was " << content_msg;
			exit(1);
		}

		LogDebug() << "Module: |" << module_name << "| syn: |" << syn_part << "| intent: |" << intent_name << "|. Sending akg";

		list_of_intents.insert(intent_name);

		(void)zmq_rep_socket_->send(zmq::str_buffer("akg"));

		subs--;
	}
	LogInfo() << "Modules ready, Sending ready signal";

	t1.join();

	zmq::message_t ready(std::string("MODULES:ready"));
	zmq_pub_socket_->send(ready, zmq::send_flags::none);

}

void FrameworkManager::StartAvA() {
	
	SaySsml(ConstStr::welcome, true);

	bool turn_off = false;
	while (!turn_off) {

		ListenForWakeUpWord();
		module_communication_->Pause();
		SayText("What can I do for you sir?", true);

		std::unique_ptr<IIntent> intent(GetIntent());
		if (intent->GetModule() == "AVA") {
			turn_off = ProcessAvaCommand(move(intent));
		}
		else {
			ProcessIntent(move(intent));
		}

		module_communication_->Resume();
		
	}
	SayText("Goodbye sir");

}

bool FrameworkManager::ProcessAvaCommand(std::unique_ptr<IIntent> intent) {
	LogDebug() << "Got ava command. action: " << intent->GetAction();
	bool turn_off = false;
	if (intent->GetAction() == "turnoff") {
		turn_off = true;
	}
	else if (intent->GetAction() == "reload_modules") {
		SayText("Reloading modules!");
		zmq::message_t stop{ std::string{"MODULES_stop"} };
		zmq_pub_socket_->send(stop, zmq::send_flags::none);
		module_service_->UnloadModules();
		LoadModules();
		SayText("Modules have been reloaded");
	}
	else if (intent->GetAction() == "cancel_mod_comm") {
		SayText("On it!");
		module_communication_->Cancel();
	}
	intent.reset();
	return turn_off;
}

void FrameworkManager::ProcessIntent(std::unique_ptr<IIntent> intent) {
	LogDebug() << "Broadcasting intent: " << intent->ToString();
	BroadCastIntent(*intent);
	bool is_done = false;
	while (!is_done) {
		LogDebug() << "Waiting on modules response";
		zmq::message_t msg;
		(void)zmq_rep_socket_->recv(msg, zmq::recv_flags::none);

		if (msg.empty()) {
			LogWarn() << "Got no response from module.";
			is_done = true;
		}
		else {
			zmq::message_t response = module_communication_->ProcessModuleMsg(msg);
			is_done = response.to_string() == "done";
			zmq_rep_socket_->send(response, zmq::send_flags::none);
		}
		
	}
	intent.reset();
}


void FrameworkManager::BroadCastIntent(IIntent& intent) {
	zmq::message_t intent_msg(intent.ToString());
	zmq_pub_socket_->send(intent_msg, zmq::send_flags::none);
}

void FrameworkManager::ListenForWakeUpWord() {
	wake_up_service_->WaitForWakeUp();
}

string FrameworkManager::GetText(int seconds) {
	return speech_to_text_service_->GetText(seconds);
}

void FrameworkManager::SayText(string msg, bool async, string language) {
	if (async) text_to_speech_service_->SayTextAsync(msg, language);
	else text_to_speech_service_->SayText(msg, language);
}

void FrameworkManager::SaySsml(string msg, bool async, string language) {
	if (async) text_to_speech_service_->SaySsmlAsync(msg, language);
	else text_to_speech_service_->SaySsml(msg, language);
}

IIntent* FrameworkManager::GetConfirmation() {
	return speech_to_intent_service_->GetConfirmation();
}

IIntent* FrameworkManager::GetIntent() {
	IIntent* intent = nullptr;
	while (!intent) {
		intent = speech_to_intent_service_->GetIntent();
		if (!intent) {
			SayText("Sorry, I couldn't get that. Could you repeat again, sir?");
		}
	}
	return intent;
}

FrameworkManager::~FrameworkManager() {
	LogInfo() << "Shutting-down ava framework...";
	zmq::message_t stop{ std::string{"MODULES_stop"} };
	zmq_pub_socket_->send(stop, zmq::send_flags::none);
	
	module_listener_->Stop();
	module_communication_->Stop();
	listener_thread_.join();
	communication_thread_.join();

	delete module_service_;
	delete module_listener_;
	delete module_communication_;
	delete zmq_pub_socket_;
	delete zmq_rep_socket_;
	delete zmq_context_;
	delete text_to_speech_service_;
	delete wake_up_service_;
	delete speech_to_intent_service_;
	delete speech_to_text_service_;
	LogInfo() << "Shutdown complete. Bye!";
};

