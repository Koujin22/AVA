#pragma once
#include <memory>
#include <queue>
#include "Logging.hpp"
#include <vector>


namespace zmq {
	class context_t;
	class socket_t;
	class message_t;
}

class ModuleRequest;
class ModuleRequestComparator;
class ModuleCommunicationService;

class ModuleListenerService : private LoggerFactory {
public:
	ModuleListenerService(zmq::context_t&, ModuleCommunicationService&);

	void Start();

	void Stop();

	~ModuleListenerService();

private:
	zmq::socket_t* const zmq_rep_socket_;
	static volatile bool status_;
	ModuleCommunicationService& module_communication_;

};
