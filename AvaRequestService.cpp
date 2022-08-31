#include "AvaRequestService.hpp"
#include "ModuleRequest.hpp"

AvaRequestService::AvaRequestService() : LoggerFactory(this), request_queue_{} {}

void AvaRequestService::Notify() {
	condition_variable_.notify_one();
}

void AvaRequestService::Notify(ModuleRequest& module_req) {
	Add(module_req);
	condition_variable_.notify_one();
}

void AvaRequestService::Add(ModuleRequest& module_req) {
	std::unique_lock lock(mutex_);

	request_queue_.push(module_req);

}

void AvaRequestService::Cancel() {
	std::unique_lock lock(mutex_);
	LogInfo() << "Cancelling... " << request_queue_.size();
	if (request_queue_.empty()) return;
	cancel_queue_.push(request_queue_.top());
	request_queue_.pop();
}

ModuleRequest AvaRequestService::GetCancel() {
	std::unique_lock lock(mutex_);
	ModuleRequest r;
	if (cancel_queue_.empty()) {
		return r;
	}
	r = cancel_queue_.top();
	cancel_queue_.pop();
	return r;
	
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

