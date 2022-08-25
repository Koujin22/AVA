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
		LogInfo() << "Starting framework manager...";
		LogVerbose() << "Binding pub and rep sockets.";
		zmq_pub_socket_->bind("tcp://127.0.0.1:5500");
		zmq_rep_socket_->bind("tcp://127.0.0.1:5501");
		zmq_rep_socket_->set(zmq::sockopt::rcvtimeo, 5000);
		LogInfo() << "Starting modules...";


		int subs = module_service_->CountModules();

		std::thread t1(&IModuleService::LoadModules, module_service_);

		std::unordered_set <string> list_of_intents;

		LogInfo() << "Connecting to modules...";
		std::string msg;
		while (subs > 0) {
			zmq::message_t msg;
			zmq::recv_result_t result = zmq_rep_socket_->recv(msg);
			if (msg.empty()) {
				LogError() << "Module connection timeout!";
				exit(1);
			}
			std::string content_msg = msg.to_string();

			if (content_msg.find(":") == -1 || content_msg.find("#") == -1) {
				LogError() << "Modules must have the following pattern. <name>:syn#<intent> message recv was " << content_msg;
				exit(1);
			}

			std::string_view module_name{ content_msg.data(), content_msg.find(":") };
			std::string_view syn_part{ content_msg.data() + content_msg.find(":")+1,   content_msg.find("#") - content_msg.find(":") -1};
			std::string intent_name = content_msg.substr(content_msg.find("#") + 1, content_msg.size() - content_msg.find("#") );

			if (list_of_intents.find(intent_name) != list_of_intents.end()) {
				LogError() << "Found duplicate modules listening to the same intent category. Intent: " << intent_name;
				exit(1);
			}
			else if (syn_part != "syn" || module_name.size() == 0 || intent_name.size() == 0) {
				LogError() << "Modules must have the following pattern. <name>:syn#<intent> message recv was " << content_msg;
				exit(1);
			} 

			LogDebug() << "Module: |" << module_name << "| syn: |" << syn_part << "| intent: |" << intent_name << "|. Sending akg";

			list_of_intents.insert(intent_name);
			
			(void)zmq_rep_socket_->send(zmq::str_buffer("akg"));
			
			subs--;
		}
		LogInfo() << "Modules ready";
		
		t1.join();
	}
	catch(zmq::error_t &t) {
		LogError() << t.what();
		exit(1);
	}

};

void FrameworkManager::StartAvA() {
	
	SaySsml(ConstStr::welcome, true);

	bool turn_off = false;
	bool understood = false;
	while (!turn_off) {

		ListenForWakeUpWord();

		SayText("What can I do for you sir?", true);
		understood = false;
		while (!understood) {
			std::unique_ptr<IIntent> intent(GetIntent());
			
			if (!intent) {
				SayText("Sorry, I couldn't get that. Could you repeat again, sir?", true);
			}
			else if (intent->GetModule() == "AVA" ){
				understood = true;
				if (intent->GetAction() == "turnoff") {
					turn_off = true;
				}
				else {}
			}
			else {
				//LogWarn() << "Intent understod yet it is not implemented yet.";
				LogDebug() << "Broadcasting intent: " << intent->ToString();
				BroadCastIntent(*intent);
				LogDebug() << "Waiting on modules response";
				zmq::message_t msg;
				(void)zmq_rep_socket_->recv(msg, zmq::recv_flags::none);
				if (msg.empty()) {
					LogWarn() << "Got not response from module.";
				}
				else {

					std::string_view msg_view = msg.to_string_view();
					LogDebug() << "Got msg back. Msg: |" << msg_view << "|";
					size_t end_command = msg_view.find("_");
					if (end_command == -1) {
						LogVerbose() << "No action required from ava framework. Continuing";
					}
					else {
						std::string_view command{ msg_view.data(), end_command };
						std::string param = msg.to_string().substr(end_command + 1,  msg.to_string().find("#") - end_command-1);
						std::string vars = msg.to_string().substr(msg.to_string().find("#")+1);

						LogDebug() << "Command is: " << command << " Param is " << param<< " vars: " << vars;
						if (command == "say") {
							
							if (vars.find('A') == -1 || vars.find('L') == -1) {
								LogError() << "Say command missing vars.";
								exit(1);
							}
							bool async = vars.at(vars.find('A') + 1) == 't';
							std::string lang = vars.substr(vars.find('L') + 1);

							SayText(param, async, lang);
							zmq::message_t empty_msg{ std::string{"Done"}};
							zmq_rep_socket_->send(empty_msg, zmq::send_flags::none);
						}
						else if (command == "say-listen") {
							if (vars.find('A') == -1 || vars.find('L') == -1) {
								LogError() << "Say command missing vars.";
								exit(1);
							}
							bool async = vars.at(vars.find('A') + 1) == 't';
							std::string lang = vars.substr(vars.find('L'));

							SayText(param, async, lang);
							zmq::message_t dication{ GetText(10) };
							zmq_rep_socket_->send(dication, zmq::send_flags::none);
						}
					}
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
	LogInfo() << "Shutting-down ava framework...";
	zmq::message_t stop{ std::string{"MODULES_stop"} };
	zmq_pub_socket_->send(stop, zmq::send_flags::none);
	delete module_service_;
	delete zmq_pub_socket_;
	delete zmq_rep_socket_;
	delete zmq_context_;
	delete text_to_speech_service_;
	delete wake_up_service_;
	delete speech_to_intent_service_;
	delete speech_to_text_service_;
	LogInfo() << "Shutdown complete. Bye!";
};

