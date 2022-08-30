#pragma once
#include <memory>
#include <string>
#include "Logging.hpp"
#include <queue>
#include <thread>

class IWakeUpService;
class ITextToSpeechService;
class IMicrophoneService;
class ISpeechToIntentService;
class ISpeechToTextService;
class IIntent;
class IModuleLoaderService;
class ModuleRequest;
class ModuleRequestComparator;
class ModuleActivatedService;
class ModuleListenerService;
class ModuleCommunicationService;

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
	FrameworkManager();
	void StartAvA();
	void SayText(std::string, bool async = false, std::string = "en-us");
	void SaySsml(std::string, bool async = false, std::string = "en-us");
	IIntent* GetConfirmation();
	std::string GetText(int);
	~FrameworkManager();
private:
	void LoadModules();
	void ListenForWakeUpWord();
	bool ProcessAvaCommand(std::unique_ptr<IIntent> intent);
	IIntent* GetIntent();
	
	void ProcessIntent(std::unique_ptr<IIntent> intent);

	FrameworkManager(std::shared_ptr<IMicrophoneService>);
	//Framework resources
	ITextToSpeechService* const text_to_speech_service_;
	IWakeUpService* const  wake_up_service_;
	ISpeechToIntentService* const speech_to_intent_service_;
	ISpeechToTextService* const speech_to_text_service_;

	//Module resources
	ModuleCommunicationService* const module_communication_service_;
	ModuleActivatedService* const module_activated_service_;
	std::thread communication_thread_;
	ModuleListenerService* const module_listener_;
	std::thread listener_thread_;
	IModuleLoaderService* const module_service_;


	
};