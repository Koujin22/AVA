#include "ModuleListenerService.hpp"
#include "ModuleRequest.hpp"
#include <zmq.hpp>
#include "ModuleActivatedService.hpp"
#include "ModuleCommunicationService.hpp"

volatile bool ModuleListenerService::status_ = false;

ModuleListenerService::ModuleListenerService(ModuleActivatedService& module_activated, zmq::context_t& context) :
	LoggerFactory(this),
	zmq_rep_socket_{ new zmq::socket_t(context, ZMQ_REP) },
	module_activated_{module_activated} { 

	try {

		LogVerbose() << "Binding pub and rep sockets.";
		zmq_rep_socket_->bind("tcp://127.0.0.1:5502");
		zmq_rep_socket_->set(zmq::sockopt::rcvtimeo, 1000);
	}
	catch (zmq::error_t& t) {
		LogError() << t.what();
		exit(1);
	}
}

void ModuleListenerService::Start() {
	if (status_ == true) {
		LogError() << "Tried to start module listener when there was already one running.";
		exit(1);
	}
	status_ = true;
	while (status_) {
		zmq::message_t msg;
		(void)zmq_rep_socket_->recv(msg, zmq::recv_flags::none);
		if (msg.empty()) continue;
		//msg expected is <name>:req#<priority>
		std::string msg_content = msg.to_string();
		LogDebug() << "Got msg from module! msg: " << msg_content;
		int colon_pos = msg_content.find(":");
		int hash_pos = msg_content.find("#");

		if(colon_pos == -1 || hash_pos == -1 || colon_pos == 0 || hash_pos == msg_content.size()-1 || msg_content.substr(colon_pos + 1, 3) != "req") {
			LogWarn() << "Got invalid request from module. " << msg_content;
			zmq::message_t response{ std::string{"invalid"} };
			zmq_rep_socket_->send(response, zmq::send_flags::none);
			continue;
		} 
		ModuleRequest req{ msg_content, colon_pos, hash_pos };

		
		LogInfo() << "Got valid request. Adding to queue.";
		zmq::message_t response{ std::string{"ok"} };
		zmq_rep_socket_->send(response, zmq::send_flags::none);
		module_activated_.Notify(req);

	}
}

void ModuleListenerService::Stop() {
	if (status_ == false) {
		LogWarn() << "Tried to stop when it wasnt running.";
	}
	status_ = false;
}

ModuleListenerService::~ModuleListenerService() {
	zmq_rep_socket_->set(zmq::sockopt::linger, 1);
	zmq_rep_socket_->close();
	delete zmq_rep_socket_;
}