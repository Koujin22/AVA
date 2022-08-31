#include "AvaModuleRequest.hpp"
#include "ModuleRequest.hpp"
#include "AvaRequestService.hpp"
#include <zmq.hpp>


volatile bool AvaModuleRequest::status_ = false;

AvaModuleRequest::AvaModuleRequest(std::shared_ptr<zmq::context_t> context, std::shared_ptr<AvaRequestService> ava_req) :
	LoggerFactory(this),
	context_{ context },
	ava_req_{ava_req},
	socket_{ new zmq::socket_t(*context_, ZMQ_REP) }
{
	try {
		LogVerbose() << "Binding pub and rep sockets.";
		socket_->bind("tcp://127.0.0.1:5502");
		socket_->set(zmq::sockopt::rcvtimeo, 1000);

	}
	catch (zmq::error_t& t) {
		LogError() << t.what();
		exit(1);
	}
}

void AvaModuleRequest::Start() {
	if (status_ == true) {
		LogError() << "Tried to start module listener when there was already one running.";
		exit(1);
	}
	status_ = true;
	Run();
}

void AvaModuleRequest::Run() {
	while (status_) {
		zmq::message_t msg;
		(void)socket_->recv(msg, zmq::recv_flags::none);
		if (msg.empty()) continue;
		//msg expected is <name>:req#<priority>
		std::string msg_content = msg.to_string();
		LogDebug() << "Got msg from module! msg: " << msg_content;
		int colon_pos = msg_content.find(":");
		int hash_pos = msg_content.find("#");

		if (colon_pos == -1 || hash_pos == -1 || colon_pos == 0 || hash_pos == msg_content.size() - 1 || msg_content.substr(colon_pos + 1, 3) != "req") {
			LogWarn() << "Got invalid request from module. " << msg_content;
			zmq::message_t response{ std::string{"invalid"} };
			socket_->send(response, zmq::send_flags::none);
			continue;
		}
		ModuleRequest req{ msg_content, colon_pos, hash_pos };


		LogInfo() << "Got valid request. Adding to queue. " << req.ToString();
		zmq::message_t response{ std::string{"ok"} };
		socket_->send(response, zmq::send_flags::none);
		ava_req_->Notify(req);
		//module_activated_.Notify(req);

	}
}

void AvaModuleRequest::Stop() {
	if (status_ == false) {
		LogWarn() << "Tried to stop when it wasnt running.";
	}
	status_ = false;
}


AvaModuleRequest::~AvaModuleRequest() {
	delete socket_;
}
