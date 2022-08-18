
#include "GoogleTextToSpeechService.hpp"
#include <google/cloud/texttospeech/text_to_speech_client.h>
#include <mmsystem.h>
#include <regex>

using std::string, std::regex, std::regex_replace;

namespace texttospeech = google::cloud::texttospeech;

void GoogleTextToSpeechService::SayText(string msg, string lang) {

	LogInfo() << "Saying plain text: " << msg;

	google::cloud::texttospeech::SynthesisInput input;
	input.set_text(msg);
	Say(input, lang, false);
}

void GoogleTextToSpeechService::SaySsml(string msg, string lang) {

	LogInfo() << "Saying in ssml: " << SsmlToString(msg);

	google::cloud::texttospeech::SynthesisInput input;
	input.set_ssml(msg);
	Say(input, lang, false);
}

void GoogleTextToSpeechService::SayTextAsync(string msg, string lang) {

	LogInfo() << "Saying async plain text: " << msg;

	google::cloud::texttospeech::SynthesisInput input;
	input.set_text(msg);
	Say(input, lang, true);
}

void GoogleTextToSpeechService::SaySsmlAsync(string msg, string lang) {

	LogInfo() << "Saying async in ssml: " << SsmlToString(msg);

	google::cloud::texttospeech::SynthesisInput input;
	input.set_ssml(msg);
	Say(input, lang, true);
}

void GoogleTextToSpeechService::Say(texttospeech::SynthesisInput& input, string lang, bool async) {
	LogVerbose() << "Setting language code. Value: "<<lang;
	texttospeech::VoiceSelectionParams voice_config;
	voice_config.set_language_code(lang);
	//Set voice name
	LogVerbose() << "Setting voice name";
	if (lang == "en-us") {
		voice_config.set_name(voice_name_[0]);
	}
	else if (lang == "es-us") {
		voice_config.set_name(voice_name_[1]);
	}
	else {
		LogWarn() << "Lenguaje: " << lang << " no implementado. Usando default.";
		voice_config.set_name(voice_name_[0]);
	}
	//Set audio config
	LogVerbose() << "Setting audio encoding";
	texttospeech::AudioConfig audio_config;
	audio_config.set_audio_encoding(texttospeech::LINEAR16);

	string audio = Synthezise(input, voice_config, audio_config);

	PlayAudio(audio, async);

}

string GoogleTextToSpeechService::Synthezise(
	texttospeech::SynthesisInput& input,
	texttospeech::VoiceSelectionParams& voice,
	texttospeech::AudioConfig& audio) {
	

	//Save response

	LogVerbose() << "Synthezising";
	google::cloud::v2_1_0::StatusOr<texttospeech::v1::SynthesizeSpeechResponse> response;
	response = client_->SynthesizeSpeech(input, voice, audio);
	if (!response) throw std::runtime_error(response.status().message());
	return response->audio_content();
}

void GoogleTextToSpeechService::PlayAudio(string audio_content, bool async) {
	LogVerbose() << "Playing audio";
	if (async) {
		PlaySoundW((LPCTSTR)audio_content.c_str(), NULL, SND_MEMORY | SND_ASYNC);
	}
	else {
		PlaySoundW((LPCTSTR)audio_content.c_str(), NULL, SND_MEMORY );

	}
}

GoogleTextToSpeechService::GoogleTextToSpeechService() : LoggerFactory(this) {
	LogInfo() << "Starting text-to-speech service...";
	//Create client
	LogVerbose() << "Creating connection to googlee text-to-speech service";
	client_ = new texttospeech::TextToSpeechClient(
		texttospeech::MakeTextToSpeechConnection());

	LogInfo() << "Text-to-speech service ready!";
};

string GoogleTextToSpeechService::SsmlToString(string msg) {
	regex e("(<[^>]*>)|(\r\n|\r|\n)|(  )");
	return regex_replace(msg, e, "");
}

GoogleTextToSpeechService::~GoogleTextToSpeechService() {
	LogInfo() << "Shuting down text-to-speech service...";
	delete client_;
	LogInfo() << "Text-to-speech service has been shut down.";
}