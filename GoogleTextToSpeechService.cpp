#include "GoogleTextToSpeechService.hpp"
#include "google/cloud/texttospeech/text_to_speech_client.h"

auto constexpr kText = R"""(
Four score and seven years ago our fathers brought forth on this
continent, a new nation, conceived in Liberty, and dedicated to
the proposition that all men are created equal.)""";

using std::string;

void GoogleTextToSpeechService::Say(string msg) {

	LogDebug() << "Saying: " << msg;

	namespace texttospeech = ::google::cloud::texttospeech;
	auto client = texttospeech::TextToSpeechClient(
		texttospeech::MakeTextToSpeechConnection());

	google::cloud::texttospeech::v1::SynthesisInput input;
	input.set_text(kText);
	google::cloud::texttospeech::v1::VoiceSelectionParams voice;
	voice.set_language_code("en-US");
	google::cloud::texttospeech::v1::AudioConfig audio;
	audio.set_audio_encoding(google::cloud::texttospeech::v1::LINEAR16);

	auto response = client.SynthesizeSpeech(input, voice, audio);
	if (!response) throw std::runtime_error(response.status().message());
	// Normally one would play the results (response->audio_content()) over some
	// audio device. For this quickstart, we just print some information.
	auto constexpr kWavHeaderSize = 48;
	auto constexpr kBytesPerSample = 2;  // we asked for LINEAR16
	auto const sample_count =
		(response->audio_content().size() - kWavHeaderSize) / kBytesPerSample;
	std::cout << "The audio has " << sample_count << " samples\n";
}

void GoogleTextToSpeechService::SetupService() {
	LogDebug() << "Start tts services";
	is_setup_ = true;
}

void GoogleTextToSpeechService::EndService() {
	LogDebug() << "End tts service";
	is_setup_ = false;
}

GoogleTextToSpeechService::~GoogleTextToSpeechService() {
	if (is_setup_) {
		EndService();
	}
}