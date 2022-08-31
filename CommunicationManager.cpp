#include "CommunicationManager.hpp"
#include <zmq.hpp>
#include <unordered_set>

CommunicationManager::CommunicationManager(std::shared_ptr<zmq::context_t> context) :
	LoggerFactory{ this },
	zmq_context_ { context },
	zmq_pub_socket_{ new zmq::socket_t(*zmq_context_, ZMQ_PUB) },
	zmq_rep_socket_{ new zmq::socket_t(*zmq_context_, ZMQ_REP) }
{
	try {
		LogInfo() << "Starting framework manager...";
		LogVerbose() << "Binding pub and rep sockets.";
		zmq_pub_socket_->bind("tcp://127.0.0.1:5500");
		zmq_rep_socket_->bind("tcp://127.0.0.1:5501");
		zmq_rep_socket_->set(zmq::sockopt::rcvtimeo, 5000);

	}
	catch (zmq::error_t& t) {
		LogError() << t.what();
		exit(1);
	}
};


void CommunicationManager::PublishMsg(zmq::message_t& msg) {
	(void)zmq_pub_socket_->send(msg, zmq::send_flags::none);
}

void CommunicationManager::SendMsg(zmq::message_t& msg) {
	(void)zmq_rep_socket_->send(msg, zmq::send_flags::none);
}

void CommunicationManager::RecvMsg(zmq::message_t& msg) {
	(void)zmq_rep_socket_->recv(msg, zmq::recv_flags::none);
}

void CommunicationManager::WaitForSynModules(int count) {
	LogInfo() << "Connecting to modules...";
	std::unordered_set<std::string> list_of_intents;
	std::string msg;
	while (count > 0) {
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

		count--;
	}
}


CommunicationManager::~CommunicationManager() {
	zmq::message_t stop{ std::string{"MODULES_stop"} };
	zmq_pub_socket_->send(stop, zmq::send_flags::none);

	delete zmq_pub_socket_;
	delete zmq_rep_socket_;
}
