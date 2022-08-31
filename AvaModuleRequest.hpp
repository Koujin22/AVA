#pragma once
#include "Logging.hpp"

namespace zmq {
	class socket_t;
	class context_t;
}

class AvaRequestService;

class AvaModuleRequest : private LoggerFactory {
public:
	AvaModuleRequest(std::shared_ptr<zmq::context_t>, std::shared_ptr<AvaRequestService>);

	void Start();
	void Run();

	void Stop();

	~AvaModuleRequest();

private:

	std::shared_ptr<zmq::context_t> context_;
	std::shared_ptr<AvaRequestService> ava_req_;
	zmq::socket_t* const socket_;
	static volatile bool status_;

};
