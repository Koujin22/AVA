#pragma once
#include <memory>
#include <string>

class IWakeUpService;
class ITextToSpeechService;
class IMicrophoneService;
class ISpeechToIntentService;
class ISpeechToTextService;
class IIntent;


/*
* FremworkService se encarga de manejar y coordinar los servicios basicos para que AVA funcione.
*/
class FrameworkManager {
public: 
	FrameworkManager();
	void ListenForWakeUpWord();
	void SayText(std::string);
	void SaySsml(std::string);
	std::string GetText(int);
	IIntent* GetIntent();
	~FrameworkManager();
private:
	FrameworkManager(std::shared_ptr<IMicrophoneService>);
	ITextToSpeechService* const text_to_speech_service_;
	IWakeUpService* const  wake_up_service_;
	ISpeechToIntentService* const speech_to_intent_service_;
	ISpeechToTextService* const speech_to_text_service_;

};