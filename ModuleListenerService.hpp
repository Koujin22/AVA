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
	ModuleListenerService(ModuleActivatedService&, ModuleCommunicationService&);

	void Start();

	void Stop();
private:
	static volatile bool status_;
	ModuleCommunicationService& module_communication_;
	ModuleActivatedService& module_activated_;


};
