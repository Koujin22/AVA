#include "FrameworkManager.hpp"
#include "PicoWakeUpService.hpp"
#include "GoogleTextToSpeechService.hpp"
#include "PicoRecorderService.hpp"
#include "PicoSpeechToIntentService.hpp"

using std::string;

FrameworkManager::FrameworkManager() : FrameworkManager(std::make_shared<PicoRecorderService>()) {};

FrameworkManager::FrameworkManager(std::shared_ptr<IMicrophoneService> microphone_service) :
	text_to_speech_service_{ new GoogleTextToSpeechService() },
	wake_up_service_{ new PicoWakeUpService(microphone_service) },
	speech_to_intent_service_{ new PicoSpeechToIntentService(microphone_service) } {};

void FrameworkManager::ListenForWakeUpWord() {
	wake_up_service_->WaitForWakeUp();
}

void FrameworkManager::SayText(string msg) {
	text_to_speech_service_->SayText(msg);
}

void FrameworkManager::SaySsml(string msg) {
	text_to_speech_service_->SaySsml(msg);
}

IIntent* FrameworkManager::GetIntent() {
	return speech_to_intent_service_->GetIntent();
}

FrameworkManager::~FrameworkManager() {
	delete text_to_speech_service_;
	delete wake_up_service_;
	delete speech_to_intent_service_;
};

