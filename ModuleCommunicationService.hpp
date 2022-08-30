#pragma once
#include "Logging.hpp"
#include <mutex>
#include <condition_variable>

class FrameworkManager;

namespace zmq {
	class message_t;
	class socket_t;
	class context_t;
}

class IIntent;

class ModuleCommunicationService : private LoggerFactory {
public:
	ModuleCommunicationService(FrameworkManager& framework);

	void RecvMsgFromModule(zmq::message_t&);
	void SendMsgToModule(zmq::message_t&);
	void BroadCastMsg(zmq::message_t&);
	void BroadCastIntent(std::unique_ptr<IIntent> intent);

	zmq::message_t ProcessModuleMsg(zmq::message_t&);

	~ModuleCommunicationService();

private:

	FrameworkManager& framework_;
	std::mutex mutex_;
	std::condition_variable condition_variable_;

	zmq::context_t* const zmq_context_;
	zmq::socket_t* const zmq_pub_socket_;
	zmq::socket_t* const zmq_rep_socket_;
};