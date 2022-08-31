#include "AvaRequestService.hpp"
#include "ModuleRequest.hpp"

AvaRequestService::AvaRequestService() : LoggerFactory(this), request_queue_{} {}

void AvaRequestService::Notify(ModuleRequest& module_req) {
	std::unique_lock lock(mutex_);

	request_queue_.push(module_req);
	lock.unlock();
	condition_variable_.notify_one();
}

ModuleRequest AvaRequestService::WaitForRequest() {
	std::unique_lock lock(mutex_);

	if (condition_variable_.wait_until(lock, std::chrono::system_clock::now() + std::chrono::seconds(1), [&] {return !request_queue_.empty(); })) {
		ModuleRequest t = request_queue_.top();
		request_queue_.pop();
		return t;
	}
	else {
		throw TimeoutError();
	}

}