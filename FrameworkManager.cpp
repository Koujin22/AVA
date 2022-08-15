#include "FrameworkManager.hpp"
#include "PicoWakeUpService.hpp"
#include "GoogleTextToSpeechService.hpp"

using std::string;

FrameworkManager::FrameworkManager() : wake_up_service_{new PicoWakeUpService()}, text_to_speech_service_{new GoogleTextToSpeechService()} {};

void FrameworkManager::Setup() {
	wake_up_service_->SetupService();
	text_to_speech_service_->SetupService();
}

void FrameworkManager::ListenForWakeUpWord() {
	wake_up_service_->WaitForWakeUp();
}

void FrameworkManager::Say(string msg) {
	text_to_speech_service_->Say(msg);
}

FrameworkManager::~FrameworkManager() {
	delete wake_up_service_;
	delete text_to_speech_service_;
};

