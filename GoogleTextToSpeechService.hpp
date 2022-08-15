#pragma once
#include "ITextToSpeechService.hpp"
#include "Logging.hpp"


namespace google::cloud::texttospeech{
	inline namespace v2_1_0 {
		class TextToSpeechClient;
	}
	inline namespace v1 {
		class SynthesisInput;
		class VoiceSelectionParams;
		class AudioConfig;
	}
}

namespace texttospeech = google::cloud::texttospeech;

class GoogleTextToSpeechService : public ITextToSpeechService, private LoggerFactory {
public:
	GoogleTextToSpeechService();
	void SayText(std::string);
	void SaySsml(std::string);
	~GoogleTextToSpeechService();
protected:
	std::string SsmlToString(std::string);
private:
	void Say(texttospeech::SynthesisInput&);
	void PlayAudio(std::string);
	std::string Synthezise(
		texttospeech::SynthesisInput&,
		texttospeech::VoiceSelectionParams&,
		texttospeech::AudioConfig&);
	
	texttospeech::TextToSpeechClient* client_;
	std::string language_code_ = "en-US";
	std::string voice_name_ = "en-US-Wavenet-F";

};
