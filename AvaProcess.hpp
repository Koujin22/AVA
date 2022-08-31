#pragma once
#include "Logging.hpp"
#include <mutex>
#include "AvaCommandResult.hpp"

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

	std::unique_ptr<AvaCommandResult> Run();
	std::unique_ptr<AvaCommandResult> Run(ModuleRequest&);

	void CancelComs(ModuleRequest&);

	void SaySsml(std::string, bool async = false);

	~AvaProcess();
private:

	std::unique_ptr<AvaCommandResult> RunSynchronized(std::unique_ptr<IIntent>);

	std::unique_ptr<AvaCommandResult> ProcessAvaCommand(std::unique_ptr<IIntent>);
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

