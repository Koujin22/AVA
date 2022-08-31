#pragma once
#include <memory>
#include <string>
#include "Logging.hpp"

class IWakeUpService;
class ITextToSpeechService;
class IMicrophoneService;
class ISpeechToIntentService;
class ISpeechToTextService;
class IIntent;

namespace zmq {
	class context_t;
	class socket_t;
	class message_t;
}

/*
* FremworkService se encarga de manejar y coordinar los servicios basicos para que AVA funcione.
*/
class FrameworkManager : private LoggerFactory {
public:
	FrameworkManager(std::shared_ptr<IMicrophoneService>);


	void SayText(std::string, bool async = false, std::string = "en-us");
	void SaySsml(std::string, bool async = false, std::string = "en-us");
	IIntent* GetConfirmation();
	std::string GetText(int);
	IIntent* GetIntent();

	~FrameworkManager();
private:

	ITextToSpeechService* const text_to_speech_service_;
	ISpeechToIntentService* const speech_to_intent_service_;
	ISpeechToTextService* const speech_to_text_service_;
	
};