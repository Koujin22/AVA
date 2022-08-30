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
class ModuleActivatedService;
class ModuleCommunicationService;

class ModuleListenerService : private LoggerFactory {
public:
	ModuleListenerService(ModuleActivatedService&, zmq::context_t&);

	void Start();

	void Stop();

	~ModuleListenerService();
private:
	static volatile bool status_;
	ModuleActivatedService& module_activated_;

	zmq::socket_t* const zmq_rep_socket_;

};
