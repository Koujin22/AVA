#pragma once
#include <string>

class IWakeUpService;
class ITextToSpeechService;

/*
* FremworkService se encarga de manejar y coordinar los servicios basicos para que AVA funcione.
*/
class FrameworkManager {
public: 
	FrameworkManager();
	void Setup();
	void ListenForWakeUpWord();
	void Say(std::string msg);
	~FrameworkManager();
	

private:
	IWakeUpService* const  wake_up_service_;
	ITextToSpeechService* const text_to_speech_service_;

};