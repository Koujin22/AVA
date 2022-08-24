#include "FrameworkManager.hpp"
#include "IMicrophoneService.hpp"
#include "GoogleTextToSpeechService.hpp"
#include "GoogleSpeechToTextService.hpp"
#include "PicoSpeechToIntentService.hpp"
#include "PicoWakeUpService.hpp"
#include "PicoRecorderService.hpp"
#include "ModuleService.hpp"
#include "IIntent.hpp"
#include <zmq.hpp>
#include <thread>
#include <unordered_set>
#include <chrono>
#include <string_view>

using std::string;

namespace ConstStr {
	const string welcome =
		R"(<speak>
            Ava 1.0 <break time="200ms"/>
            <emphasis level="strong">ready for action!</emphasis>
        </speak>)";
}

FrameworkManager::FrameworkManager() : 
	FrameworkManager(std::make_shared<PicoRecorderService>()) {};
	
FrameworkManager::FrameworkManager(std::shared_ptr<IMicrophoneService> microphone_service) :
	LoggerFactory(this),
	text_to_speech_service_{ new GoogleTextToSpeechService() },
	wake_up_service_{ new PicoWakeUpService(microphone_service) },
	speech_to_intent_service_{ new PicoSpeechToIntentService(microphone_service) },
	speech_to_text_service_{ new GoogleSpeechToTextService(microphone_service) },
	zmq_context_{ new zmq::context_t(1) },
	zmq_pub_socket_{ new zmq::socket_t(*zmq_context_, ZMQ_PUB) },
	zmq_rep_socket_{ new zmq::socket_t(*zmq_context_, ZMQ_REP) },
	module_service_{ new ModuleService() }
{ 

	try {
		LogDebug() << "Binding pub and rep sockets.";
		zmq_pub_socket_->bind("tcp://127.0.0.1:5500");
		zmq_rep_socket_->bind("tcp://127.0.0.1:5501");

		std::thread t1(&IModuleService::LoadModules, module_service_);
		//std::thread t2(&IModuleService::LoadModules, module_service_);

		std::unordered_set <string> list_of_intents;

		int subs = 0;
		std::string msg;
		while (subs < 2) {
			zmq::message_t msg;
			(void)zmq_rep_socket_->recv(msg);
			std::string content_msg = msg.to_string();

			LogDebug() << "Recieved msg: " << content_msg << ". Sending akg back. ";

			if (msg.to_string_view().size() <= 4 || std::string_view{ content_msg.data() + 0, 4 } != "syn_") {
				LogError() << "Modules must have the following pattern. syn_<name_of_intent>";
				exit(1);
			} 
			std::string intent_to_listen{ content_msg.data() + 4, content_msg.size() - 4 };
			if (list_of_intents.find(intent_to_listen) != list_of_intents.end()) {
				LogError() << "Found duplicate modules listening to the same intent category";
				exit(1);
			}
			else {
				LogVerbose() << "Adding module that listening to intent: " << intent_to_listen;
				list_of_intents.insert(intent_to_listen);
			}

			(void)zmq_rep_socket_->send(zmq::str_buffer("akg"));
			LogDebug() << "Sending akg";
			subs++;
		}
		LogInfo() << "Modules ready";
		
		t1.join();
		//t2.join();
	}
	catch(zmq::error_t &t) {
		LogError() << t.what();
		exit(1);
	}

};

void FrameworkManager::StartAvA() {
	
	SaySsml(ConstStr::welcome);

	bool turn_off = false;
	bool understood = false;
	while (!turn_off) {

		ListenForWakeUpWord();

		SayText("What can I do for you sir?");
		understood = false;
		while (!understood) {
			std::unique_ptr<IIntent> intent(GetIntent());
			
			if (!intent) {
				SayText("Sorry, I couldn't get that. Could you repeat again, sir?");
			}
			else if (intent->GetModule() == "AVA" && intent->GetAction() == "turnoff") {
				understood = true;
				turn_off = true;
			}
			/*else if (intent->GetModule() == "TODO" && intent->GetAction() == "add") {
				understood = true;
				SayText("What do you want me to write down?");
				std::string dictation = GetText(10);

				SayText("Okay, is ");
				SayText(dictation, false, "es-us");
				SayText("correct ? ");

			}*/
			else {
				//LogWarn() << "Intent understod yet it is not implemented yet.";
				BroadCastIntent(*intent);
				LogInfo() << "Waiting on modules response";
				zmq::message_t msg;
				zmq_rep_socket_->recv(msg, zmq::recv_flags::none);
				std::string_view msg_view = msg.to_string_view();
				size_t end_command = msg_view.find("_");
				if (end_command == -1) {
					LogInfo() << "No action required from ava framework. Continuing";
				}
				else {
					std::string_view command{ msg_view.data(), end_command };
					LogDebug() << "Command is: " << command;
				}

				
				
				understood = true;
			}
			intent.reset();
		}
	}
	SayText("Goodbye sir");

}

void FrameworkManager::BroadCastIntent(IIntent& intent) {
	zmq::message_t intent_msg(intent.ToString());
	zmq_pub_socket_->send(intent_msg, zmq::send_flags::none);
}

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
	delete zmq_pub_socket_;
	delete zmq_rep_socket_;
	delete zmq_context_;
	delete text_to_speech_service_;
	delete wake_up_service_;
	delete speech_to_intent_service_;
	delete speech_to_text_service_;
};

