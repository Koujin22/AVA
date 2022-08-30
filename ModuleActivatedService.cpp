#include "ModuleActivatedService.hpp"
#include "FrameworkManager.hpp"
#include "ModuleRequest.hpp"
#include "ModuleCommand.hpp"
#include "ModuleCommunicationService.hpp"
#include "IIntent.hpp"
#include <zmq.hpp>

volatile bool ModuleActivatedService::status_ = false;
volatile bool ModuleActivatedService::recheck_ = false;

ModuleActivatedService::ModuleActivatedService(FrameworkManager& framework, ModuleCommunicationService& module_communication_service) :
	LoggerFactory(this),
	framework_{ framework },
	module_communication_service_ { module_communication_service },
	request_queue_{}
{ 
	
}


void ModuleActivatedService::Start() {

	if (status_ == true) {
		LogError() << "Tried to start module listener when there was already one running.";
		exit(1);
	}
	status_ = true;
	
	while (status_) {
		std::unique_lock lk{ mutex_ };
		condition_variable_.wait(lk, [&] {return !request_queue_.empty() || !status_; });
		if (!status_) break;
		ModuleRequest req = request_queue_.top();
		request_queue_.pop();
		lk.unlock();

		LogInfo() << "Got request. " << req.ToString();
		bool is_done = false;
		while (!is_done) {
			LogDebug() << "Waiting on modules response";
			zmq::message_t msg = module_communication_service_.RecvMsgFromModule();

			if (msg.empty()) {
				LogWarn() << "Got no response from module.";
				is_done = true;
			}
			else {
				zmq::message_t response = module_communication_service_.ProcessModuleMsg(msg);
				is_done = response.to_string() == "done";
				module_communication_service_.SendMsgToModule(response);
			}

		}
	}
}

void ModuleActivatedService::Notify(ModuleRequest req) {

	std::unique_lock lk{ mutex_ };
	LogDebug() << "Notify: Got lock";
	request_queue_.push(req);
	recheck_ = true;
	lk.unlock();
	condition_variable_.notify_one();
	LogDebug() << "Notifying";

}

void ModuleActivatedService::Stop() {
	if (status_ == false) {
		LogWarn() << "Tried to stop when it wasnt running.";
	}
	status_ = false;
	condition_variable_.notify_one();
	Cancel();
}

void ModuleActivatedService::Pause() {

}

void ModuleActivatedService::Resume() {

}

void ModuleActivatedService::Cancel() {

}

