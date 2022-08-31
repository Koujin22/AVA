#pragma once
#include "Logging.hpp"

namespace zmq {
	class context_t;
	class socket_t;
	class message_t;
}

class CommunicationManager : private LoggerFactory {
public:
	CommunicationManager(std::shared_ptr<zmq::context_t>);

	void PublishMsg(zmq::message_t& msg);
	void SendMsg(zmq::message_t& msg);
	void RecvMsg(zmq::message_t& msg);

	void WaitForSynModules(int);

	~CommunicationManager();

private:

	std::shared_ptr<zmq::context_t> zmq_context_;
	zmq::socket_t* const zmq_pub_socket_;
	zmq::socket_t* const zmq_rep_socket_;
};