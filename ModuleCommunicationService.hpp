#pragma once
#include "Logging.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include "ModuleRequest.hpp"

class FrameworkManager;

namespace zmq {
	class message_t;
	class socket_t;
	class context_t;
}

class ModuleCommunicationService : LoggerFactory {
public:
	ModuleCommunicationService(FrameworkManager&, zmq::context_t&);

	void Start();

	void Notify(ModuleRequest);
	void Pause();
	void Resume();
	void Cancel();

	void Stop();

	zmq::message_t ProcessModuleMsg(zmq::message_t&);

	~ModuleCommunicationService();

private:
	FrameworkManager& framework_;

	zmq::socket_t* zmq_rep_socket_;
	zmq::socket_t* zmq_pub_socket_;

	std::priority_queue<ModuleRequest, std::vector<ModuleRequest>, ModuleRequestComparator> request_queue_;
	std::mutex mutex_;
	std::condition_variable condition_variable_;
	static volatile bool status_;
	static volatile bool recheck_;
};