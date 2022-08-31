#include "FrameworkManager.hpp"
#include "IMicrophoneService.hpp"
#include "GoogleTextToSpeechService.hpp"
#include "GoogleSpeechToTextService.hpp"
#include "PicoSpeechToIntentService.hpp"
#include "PicoWakeUpService.hpp"
#include "PicoRecorderService.hpp"
#include "ModuleService.hpp"
#include "PicoIntent.hpp"
#include "ModuleCommand.hpp"
#include <zmq.hpp>
#include <thread>
#include <unordered_set>
#include <chrono>
#include <string_view>

using std::string;


	
FrameworkManager::FrameworkManager(std::shared_ptr<IMicrophoneService> microphone_service) :
	LoggerFactory(this),
	text_to_speech_service_{ new GoogleTextToSpeechService() },
	speech_to_intent_service_{ new PicoSpeechToIntentService(microphone_service) },
	speech_to_text_service_{ new GoogleSpeechToTextService(microphone_service) }
{
};

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

std::unique_ptr<IIntent> FrameworkManager::GetConfirmation() {
	return speech_to_intent_service_->GetConfirmation();
}

std::unique_ptr<IIntent> FrameworkManager::GetIntent() {
	std::unique_ptr<IIntent>  intent{ nullptr };
	while (!intent) {
		intent = speech_to_intent_service_->GetIntent();
		if (!intent) {
			SayText("Sorry, I couldn't get that. Could you repeat again, sir?");
		}
	}
	return intent;
}

FrameworkManager::~FrameworkManager() {
	LogInfo() << "Shutting-down ava framework...";
	delete text_to_speech_service_;
	delete speech_to_intent_service_;
	delete speech_to_text_service_;
	LogInfo() << "Shutdown complete. Bye!";
};

