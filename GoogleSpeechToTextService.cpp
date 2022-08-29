#include "GoogleSpeechToTextService.hpp"
#include <google/cloud/speech/speech_client.h>
#include "IMicrophoneService.hpp"


namespace speech = google::cloud::speech;


GoogleSpeechToTextService::GoogleSpeechToTextService(std::shared_ptr<IMicrophoneService> microphone_ptr) : LoggerFactory(this), microhpone_{ microphone_ptr } {
	LogInfo() << "Starting speech-to-text service...";
	LogVerbose() << "Creating connection to googlee speech-to-text service";
	client_ = new speech::SpeechClient(speech::MakeSpeechConnection());
	LogDebug() << "Google speech-to-text frame length: " << 512;
	pcm_ = static_cast<int16_t*>(malloc(512 * sizeof(int16_t)));
	LogVerbose() << "Starting pcm malloc";
	if (!pcm_) {
		LogError() << "Failed to allocate pcm memory.";
		exit(1);
	}
	LogInfo() << "Speech-to-text service ready!";
}

std::string GoogleSpeechToTextService::Record(int seconds) {
	LogVerbose() << "Recording for " << seconds << " seconds.";
	int64_t max = 40 * seconds;
	std::stringstream data;
	for (int64_t i = 0; i < max; i++) {

		microhpone_->GetPcm(pcm_);

		for (int64_t j = 0; j < 512; j++) {
			data.write((char*)&pcm_[j], 2);
		}
	}

	return data.str();
}

std::string GoogleSpeechToTextService::MakeFile(std::string& raw_pcm, int seconds) {
	LogVerbose() << "Creating wave file in-memory";
	const char* riff = "RIFF";
	int32_t num = 512 * 2 * 40 * seconds;
	const char* wave = "WAVEfmt ";
	//               |   size of next part  |  PCM = 16 | chanels 1 | 
	char empty[8] = { 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 };
	//sampRate  16000       
	int32_t sampRate = 16000;
	//byterate
	int32_t byteRate = sampRate * 2;
	//               | numChl*2 |   16       |
	char form[4] = { 0x02, 0x00, 0x10, 0x00 };
	const char* data = "data";
	char size[4] = { 0x00, 0x90, 0x10, 0x00 };
	std::stringstream file;
	file.write(riff, 4);
	file.write((char*)&num, 4);
	file.write(wave, 8);
	file.write(empty, 8);
	file.write((char*)&sampRate, 4);
	file.write((char*)&byteRate, 4);
	file.write(form, 4);
	file.write(data, 4);
	file.write(size, 4);
	file << raw_pcm;

	return file.str();
}

std::string GoogleSpeechToTextService::GetText(int seconds) {
	LogInfo() << "Listening for audio to converrt into text for " << seconds << " seconds.";
	std::string raw_data = Record(seconds);
	std::string wav_file = MakeFile(raw_data, seconds);
	LogVerbose() << "Setting audio and recognition config";
	speech::v1::RecognitionConfig config;
	config.set_language_code("es-us");
	speech::v1::RecognitionAudio audio;
	audio.set_content(wav_file);

	LogVerbose() << "Sending audio to google to process";
	google::cloud::v2_1_0::StatusOr<speech::v1::RecognizeResponse> response;
	response = client_->Recognize(config, audio);

	if (!response || !response.ok()) throw std::runtime_error(response.status().message());
	
	if (response->results_size() == 0) {
		LogInfo() << "Empty dictation. Returning empty string";
		return "";
	}

	LogDebug() << "Debug info: " << response->DebugString();
	std::string result = response->results().Get(0).alternatives(0).transcript();
	LogInfo() << "Process audio: " << result;
	return result;
}

GoogleSpeechToTextService::~GoogleSpeechToTextService() {

	LogInfo() << "Shuting down speech-to-text service...";
	delete client_;
	LogVerbose() << "Freeing pcm...";
	free(pcm_);
	LogInfo() << "Speech-to-text service has been shut down.";
}
//speech-to-text 