#include "ModuleCommunicationService.hpp"
#include "ModuleCommand.hpp"
#include "IIntent.hpp"
#include "FrameworkManager.hpp"
#include <zmq.hpp>
#include <memory>



ModuleCommunicationService::ModuleCommunicationService(FrameworkManager& framework, zmq::context_t& zmq_context) :
	LoggerFactory(this),
	framework_ { framework },
	zmq_pub_socket_ { new zmq::socket_t(zmq_context, ZMQ_PUB) },
	zmq_rep_socket_{ new zmq::socket_t(zmq_context, ZMQ_REP) }
{
	try{

		LogVerbose() << "Binding pub and rep sockets.";
		zmq_pub_socket_->bind("tcp://127.0.0.1:5500");
		zmq_rep_socket_->bind("tcp://127.0.0.1:5501");
		zmq_rep_socket_->set(zmq::sockopt::rcvtimeo, 5000);
	}
	catch (zmq::error_t& t) {
		LogError() << t.what();
		exit(1);
	}
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

zmq::message_t ModuleCommunicationService::RecvMsgFromModule() {
	zmq::message_t msg;
	(void) zmq_rep_socket_->recv(msg, zmq::recv_flags::none);
	return msg;
}

void ModuleCommunicationService::SendMsgToModule(zmq::message_t& msg) {
	(void)zmq_rep_socket_->send(msg, zmq::send_flags::none);

}

void ModuleCommunicationService::BroadCastMsg(zmq::message_t& msg) {
	(void)zmq_pub_socket_->send(msg, zmq::send_flags::none);

}

void ModuleCommunicationService::BroadCastIntent(std::unique_ptr<IIntent> intent) {
	zmq::message_t intent_msg(intent->ToString());
	(void)zmq_pub_socket_->send(intent_msg, zmq::send_flags::none);
}

ModuleCommunicationService::~ModuleCommunicationService() {
	zmq_pub_socket_->set(zmq::sockopt::linger, 1);
	zmq_pub_socket_->close();
	zmq_rep_socket_->set(zmq::sockopt::linger, 1);
	zmq_rep_socket_->close();
	delete zmq_pub_socket_;
	delete zmq_rep_socket_;
}