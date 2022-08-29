#include "ModuleCommunicationService.hpp"
#include "FrameworkManager.hpp"
#include "ModuleRequest.hpp"
#include "ModuleCommand.hpp"
#include "IIntent.hpp"
#include <zmq.hpp>

volatile bool ModuleCommunicationService::status_ = false;
volatile bool ModuleCommunicationService::recheck_ = false;

ModuleCommunicationService::ModuleCommunicationService(FrameworkManager& framework, zmq::context_t& context) :
	LoggerFactory(this),
	framework_{ framework },
	request_queue_{},
	zmq_rep_socket_{ new zmq::socket_t(context, ZMQ_REP ) },
	zmq_pub_socket_{ new zmq::socket_t(context, ZMQ_PUB ) }
{ 
	try {
		zmq_pub_socket_->connect("tcp://127.0.0.1:5500");
		zmq_rep_socket_->bind("tcp://127.0.0.1:5503");
		zmq_rep_socket_->set(zmq::sockopt::rcvtimeo, 5000);

	}
	catch (zmq::error_t& t) {
		LogError() << t.what();
		exit(1);
	}
}


void ModuleCommunicationService::Start() {

	if (status_ == true) {
		LogError() << "Tried to start module listener when there was already one running.";
		exit(1);
	}
	status_ = true;
	
	while (status_) {
		std::unique_lock lk{ mutex_ };
		condition_variable_.wait(lk, [&] {return !request_queue_.empty() || !status_; });
		if (!status_) break;
		ModuleRequest req = request_queue_.top();
		request_queue_.pop();
		lk.unlock();

		LogInfo() << "Got request. " << req.ToString();
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
				zmq::message_t response = ProcessModuleMsg(msg);
				is_done = response.to_string() == "done";
				zmq_rep_socket_->send(response, zmq::send_flags::none);
			}

		}
	}
}

void ModuleCommunicationService::Notify(ModuleRequest req) {

	std::unique_lock lk{ mutex_ };
	LogDebug() << "Notify: Got lock";
	request_queue_.push(req);
	recheck_ = true;
	lk.unlock();
	condition_variable_.notify_one();
	LogDebug() << "Notifying";

}

void ModuleCommunicationService::Stop() {
	if (status_ == false) {
		LogWarn() << "Tried to stop when it wasnt running.";
	}
	status_ = false;
	condition_variable_.notify_one();
	Cancel();
}

void ModuleCommunicationService::Pause() {

}

void ModuleCommunicationService::Resume() {

}

void ModuleCommunicationService::Cancel() {

}


zmq::message_t ModuleCommunicationService::ProcessModuleMsg(zmq::message_t& msg) {
	std::string_view msg_view = msg.to_string_view();
	LogDebug() << "Got msg back. Msg: |" << msg_view << "|";
	if (msg_view == "done") {
		LogVerbose() << "No action required from ava framework. Continuing";
		zmq::message_t done{ std::string{"done"} };
		return done;
	}
	else {
		ModuleCommand mod_command(msg_view);

		LogDebug() << mod_command.ToString();
		if (mod_command.GetCommand() == "say") {

			framework_.SayText(mod_command.GetParam(), mod_command.IsAsync(), mod_command.GetLang());
			zmq::message_t empty_msg{ std::string{"okay"} };
			return empty_msg;
		}
		else if (mod_command.GetCommand() == "say-listen") {

			framework_.SayText(mod_command.GetParam(), mod_command.IsAsync(), mod_command.GetLang());
			zmq::message_t dication(framework_.GetText(mod_command.GetTime()));
			LogDebug() << "Sending dictation: " << dication.str();
			return dication;
		}
		else if (mod_command.GetCommand() == "confirm") {
			framework_.SayText(mod_command.GetParam(), mod_command.IsAsync(), mod_command.GetLang());
			std::unique_ptr<IIntent> intent{ framework_.GetConfirmation() };
			if (intent == nullptr) {
				zmq::message_t confirmation(std::string{ "" });
				return confirmation;
			}
			else {
				zmq::message_t confirmation(intent->GetAction());
				return confirmation;
			}
		}
	}
}


ModuleCommunicationService::~ModuleCommunicationService() {
	delete zmq_rep_socket_;
	delete zmq_pub_socket_;
}