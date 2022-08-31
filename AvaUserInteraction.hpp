#pragma once
#include "Logging.hpp"

class AvaProcess;
class IWakeUpService;
class IModuleService;
class IMicrophoneService;

class AvaUserInteraction : private LoggerFactory {
public:
	AvaUserInteraction(std::shared_ptr<AvaProcess>, std::shared_ptr<IMicrophoneService>);

	void Start();
	void Run();

	~AvaUserInteraction();
private:

	void ListenForWakeUpWord();


	IWakeUpService* const wake_up_service_;
	std::shared_ptr<AvaProcess> ava_;

};