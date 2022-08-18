#pragma once
#include "ITextToSpeechService.hpp"
#include "Logging.hpp"

namespace google{
	namespace cloud {
		namespace texttospeech {
			inline namespace v2_1_0 {
				class TextToSpeechClient;
			}
			inline namespace v1 {
				class SynthesisInput;
				class VoiceSelectionParams;
				class AudioConfig;
			}
		}
	}
}


class GoogleTextToSpeechService : public ITextToSpeechService, private LoggerFactory {
public:
	GoogleTextToSpeechService();
	void SayText(std::string, std::string);
	void SaySsml(std::string, std::string);
	void SayTextAsync(std::string, std::string);
	void SaySsmlAsync(std::string, std::string);
	~GoogleTextToSpeechService();
protected:
	std::string SsmlToString(std::string);
private:
	void Say(google::cloud::texttospeech::v1::SynthesisInput&, std::string, bool async);
	void PlayAudio(std::string, bool async);
	std::string Synthezise(
		google::cloud::texttospeech::v1::SynthesisInput&,
		google::cloud::texttospeech::v1::VoiceSelectionParams&,
		google::cloud::texttospeech::v1::AudioConfig&);
	
	google::cloud::texttospeech::v2_1_0::TextToSpeechClient* client_;
	std::string language_code_ = "en-US";
	std::string voice_name_ = "en-US-Wavenet-F";

};
