#pragma once
#include "ISpeechToTextService.hpp"
#include "Logging.hpp"
#include <memory>

class IMicrophoneService;

namespace google {
	namespace cloud {
		namespace speech {
			inline namespace v2_1_0 {
				class SpeechClient;
			}
		}
	}
}

class GoogleSpeechToTextService : public ISpeechToTextService, private LoggerFactory {

public:
	GoogleSpeechToTextService(std::shared_ptr<IMicrophoneService>);
	std::string GetText(int);
	~GoogleSpeechToTextService();
private:
	std::string MakeFile(std::string&, int);
	std::string Record(int);

	int16_t* pcm_ = nullptr;
	std::shared_ptr<IMicrophoneService> microhpone_;
	google::cloud::speech::v2_1_0::SpeechClient* client_;
	
};