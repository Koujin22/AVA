
#include "GoogleTextToSpeechService.hpp"
#include <google/cloud/texttospeech/text_to_speech_client.h>
#include <mmsystem.h>
#include <Windows.h>
#include <regex>

using std::string, std::regex, std::regex_replace;

namespace texttospeech = google::cloud::texttospeech;

void GoogleTextToSpeechService::SayText(string msg) {

	LogInfo() << "Saying plain text: " << msg;

	google::cloud::texttospeech::SynthesisInput input;
	input.set_text(msg);
	Say(input);
}

void GoogleTextToSpeechService::SaySsml(string msg) {

	LogInfo() << "Saying in ssml: " << SsmlToString(msg);

	google::cloud::texttospeech::SynthesisInput input;
	input.set_ssml(msg);
	Say(input);
}

void GoogleTextToSpeechService::Say(texttospeech::SynthesisInput& input) {
	LogDebug() << "Configuring synthesizer";
	LogVerbose() << "Setting language code.";
	texttospeech::VoiceSelectionParams voice_config;
	voice_config.set_language_code(language_code_);
	//Set voice name
	LogVerbose() << "Setting voice name";
	voice_config.set_name(voice_name_);
	//Set audio config
	LogVerbose() << "Setting audio encoding";
	texttospeech::AudioConfig audio_config;
	audio_config.set_audio_encoding(texttospeech::LINEAR16);

	string audio = Synthezise(input, voice_config, audio_config);

	PlayAudio(audio);

}

string GoogleTextToSpeechService::Synthezise(
	texttospeech::SynthesisInput& input,
	texttospeech::VoiceSelectionParams& voice,
	texttospeech::AudioConfig& audio) {
	

	//Save response
	LogDebug() << "Synthezising";
	google::cloud::v2_1_0::StatusOr<texttospeech::v1::SynthesizeSpeechResponse> response;
	response = client_->SynthesizeSpeech(input, voice, audio);
	if (!response) throw std::runtime_error(response.status().message());
	return response->audio_content();
}

void GoogleTextToSpeechService::PlayAudio(string audio_content) {
	LogDebug() << "Playing audio";
	PlaySoundW((LPCTSTR)audio_content.c_str(), NULL, SND_MEMORY | SND_ASYNC);
}

GoogleTextToSpeechService::GoogleTextToSpeechService() : LoggerFactory(this) {
	LogInfo() << "Starting text-to-speech service...";
	//Create client
	LogDebug() << "Creating connection to googlee text-to-speech service";
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