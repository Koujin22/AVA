#include "AvaModuleInteraction.hpp"
#include "AvaProcess.hpp"
#include "AvaRequestService.hpp"
#include "ModuleRequest.hpp"


volatile bool AvaModuleInteraction::status_ = false;

AvaModuleInteraction::AvaModuleInteraction(std::shared_ptr<AvaProcess> ava, std::shared_ptr<AvaRequestService> ava_req) :
	LoggerFactory(this), 
	ava_{ ava },
	ava_req_{ ava_req }{}


void AvaModuleInteraction::Start() {
	if (status_ == true) {
		LogError() << "Tried to start module listener when there was already one running.";
		exit(1);
	}
	status_ = true;
	Run();
}

void AvaModuleInteraction::Run() {
	while(status_){
		try {

			ModuleRequest req = ava_req_->WaitForRequest();
			LogInfo() << "Got request on module interaction! " << req.ToString();
			try {
				ava_->Run(req);
			}
			catch (ModulePaused p) {
				ava_req_->Add(req);
			}
		}
		catch (TimeoutError c) {}		
	}	
}

void AvaModuleInteraction::Stop() {
	if (status_ == false) {
		LogWarn() << "Tried to stop when it wasnt running.";
	}
	status_ = false;
}

const char* ModulePaused::what() {
	return "Module coms was paused";
}