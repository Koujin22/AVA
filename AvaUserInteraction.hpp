#pragma once
#include "Logging.hpp"

class AvaProcess;
class IWakeUpService;
class IMicrophoneService;
class AvaRequestService;

class AvaUserInteraction : private LoggerFactory {
public:
	AvaUserInteraction(std::shared_ptr<AvaProcess>, std::shared_ptr<IMicrophoneService>, std::shared_ptr<AvaRequestService>);

	void Start();
	void Run();

	~AvaUserInteraction();
private:

	void ListenForWakeUpWord();


	IWakeUpService* const wake_up_service_;
	std::shared_ptr<AvaProcess> ava_;
	std::shared_ptr<AvaRequestService> ava_req_;

};