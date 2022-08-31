#pragma once
#include "Logging.hpp"


class AvaUserInteraction;
class AvaModuleRequest;
class AvaModuleInteraction;
class AvaRequestService;


class AVA : private LoggerFactory {
public:
	AVA();
	void Start();
	~AVA();
private:
	AVA(std::shared_ptr<IMicrophoneService>, std::shared_ptr<zmq::context_t>);
	AVA(std::shared_ptr<AvaProcess>, std::shared_ptr<IMicrophoneService>, std::shared_ptr<zmq::context_t>, std::shared_ptr<AvaRequestService>);

	AvaUserInteraction* const ava_user_interaction_;
	AvaModuleRequest* const ava_module_request_;
	AvaModuleInteraction* const ava_module_interaction_;
	
};
