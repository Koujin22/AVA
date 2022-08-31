#pragma once
#include <memory>
#include <string>
#include "Logging.hpp"

class ITextToSpeechService;
class IMicrophoneService;
class ISpeechToIntentService;
class ISpeechToTextService;
class IIntent;


/*
* FremworkService se encarga de manejar y coordinar los servicios basicos para que AVA funcione.
*/
class FrameworkManager : private LoggerFactory {
public:
	FrameworkManager(std::shared_ptr<IMicrophoneService>);


	void SayText(std::string, bool async = false, std::string = "en-us");
	void SaySsml(std::string, bool async = false, std::string = "en-us");
	std::unique_ptr<IIntent> GetConfirmation();
	std::string GetText(int);
	std::unique_ptr<IIntent> GetIntent();

	~FrameworkManager();
private:

	ITextToSpeechService* const text_to_speech_service_;
	ISpeechToIntentService* const speech_to_intent_service_;
	ISpeechToTextService* const speech_to_text_service_;
	
};