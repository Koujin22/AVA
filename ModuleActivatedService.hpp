#pragma once
#include "Logging.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include "ModuleRequest.hpp"

class FrameworkManager;
class ModuleCommunicationService;

namespace zmq {
	class message_t;
	class socket_t;
	class context_t;
}

class ModuleActivatedService : LoggerFactory {
public:
	ModuleActivatedService(FrameworkManager&, ModuleCommunicationService&);

	void Start();

	void Notify(ModuleRequest);

	void Pause();
	void Resume();
	void Cancel();

	void Stop();


private:
	FrameworkManager& framework_;
	ModuleCommunicationService& module_communication_service_;

	std::priority_queue<ModuleRequest, std::vector<ModuleRequest>, ModuleRequestComparator> request_queue_;
	std::mutex mutex_;
	std::condition_variable condition_variable_;
	static volatile bool status_;
	static volatile bool recheck_;
};