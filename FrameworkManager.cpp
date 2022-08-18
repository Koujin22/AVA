#include "FrameworkManager.hpp"
#include "IMicrophoneService.hpp"
#include "GoogleTextToSpeechService.hpp"
#include "GoogleSpeechToTextService.hpp"
#include "PicoSpeechToIntentService.hpp"
#include "PicoWakeUpService.hpp"
#include "PicoRecorderService.hpp"

using std::string;

FrameworkManager::FrameworkManager() : FrameworkManager(std::make_shared<PicoRecorderService>()) {};

FrameworkManager::FrameworkManager(std::shared_ptr<IMicrophoneService> microphone_service) :
	text_to_speech_service_{ new GoogleTextToSpeechService() },
	wake_up_service_{ new PicoWakeUpService(microphone_service) },
	speech_to_intent_service_{ new PicoSpeechToIntentService(microphone_service) },
	speech_to_text_service_{ new GoogleSpeechToTextService(microphone_service) } {};

void FrameworkManager::ListenForWakeUpWord() {
	wake_up_service_->WaitForWakeUp();
}


string FrameworkManager::GetText(int seconds) {
	return speech_to_text_service_->GetText(seconds);
}

void FrameworkManager::SayText(string msg, bool async, string language) {
	if (async) text_to_speech_service_->SayTextAsync(msg, language);
	else text_to_speech_service_->SayText(msg, language);
}

void FrameworkManager::SaySsml(string msg, bool async, string language) {
	if (async) text_to_speech_service_->SaySsmlAsync(msg, language);
	else text_to_speech_service_->SaySsml(msg, language);
}

IIntent* FrameworkManager::GetIntent() {
	return speech_to_intent_service_->GetIntent();
}

FrameworkManager::~FrameworkManager() {
	delete text_to_speech_service_;
	delete wake_up_service_;
	delete speech_to_intent_service_;
	delete speech_to_text_service_;
};

