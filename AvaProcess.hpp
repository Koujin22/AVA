#pragma once
#include "Logging.hpp"
#include <mutex>

class FrameworkManager;
class CommunicationManager;
class IMicrophoneService;
class ModuleService;
class IIntent;
class ModuleRequest;

namespace zmq {
	class message_t;
	class context_t;
}

class AvaProcess : private LoggerFactory {
public:
	AvaProcess(std::shared_ptr<IMicrophoneService>, std::shared_ptr<zmq::context_t>);

	bool Run();
	bool Run(ModuleRequest&);

	void SaySsml(std::string, bool async = false);

	~AvaProcess();
private:

	bool RunSynchronized(std::unique_ptr<IIntent>);

	bool ProcessAvaCommand(std::unique_ptr<IIntent>);
	void CommunicateModule(std::unique_ptr<IIntent>);
	bool ProcessModuleMsg(zmq::message_t&);

	void LoadModules();
	void ReloadModules();

	FrameworkManager* const framework_;
	CommunicationManager* const comms_;
	ModuleService* const modules_;
	
	std::mutex mutex_;

	std::mutex control_mutex_;
	bool should_pause_;
};

