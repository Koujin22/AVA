#pragma once
#include "Logging.hpp"

class AvaProcess;
class AvaRequestService;

class ModulePaused : public std::exception {
public:
	const char* what();
};

class AvaModuleInteraction : private LoggerFactory {
public:
	AvaModuleInteraction(std::shared_ptr<AvaProcess>, std::shared_ptr<AvaRequestService>);

	void Start();
	void Stop();
	void Run();

private:
	std::shared_ptr<AvaProcess> ava_;
	std::shared_ptr<AvaRequestService> ava_req_;
	static volatile bool status_;
};