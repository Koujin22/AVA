#include "GoogleSpeechToTextService.hpp"

#include <fstream>
#include <string>

using namespace std;

GoogleSpeechToTextService::GoogleSpeechToTextService() : LoggerFactory(this) {
	microphone_ = new PicoRecorderService();
}

std::string GoogleSpeechToTextService::GetText(std::string tests) {

	int64_t max = 100;
	

	int16_t** test = new int16_t * [100]();

	for (int64_t i = 0; i < max; i++) {
		int16_t* tmp = static_cast<int16_t*>(malloc(512 * sizeof(int16_t)));
		test[i] = tmp;
		microphone_->GetPcm(tmp);
		if (i % 10 == 0) {
			std::cout << i << std::endl;
		}
	}

	const char* riff = "RIFF";
	int32_t num = 512 * 2 * max;
	const char* wave = "WAVEfmt ";
	//               |   size of next part  |  PCM = 16 | chanels 1 | 
	char empty[8] = { 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00 }; 
	//sampRate  16000       
	int32_t sampRate = 16000;
	//byterate
	int32_t byteRate = sampRate * 2;
	//               | numChl*2 |   16       |
	char form [4] = { 0x02, 0x00, 0x10, 0x00 };
	const char* data = "data";
	char size[4] = { 0x00, 0x90, 0x10, 0x00 };
	ofstream file("file.wav", ios::binary);
	file.write(riff, 4);
	file.write((char*)&num, 4);
	file.write(wave, 8);
	file.write(empty, 8);
	file.write((char*)&sampRate, 4);
	file.write((char*)&byteRate, 4);
	file.write(form, 4);
	file.write(data, 4);
	file.write(size, 4);
	for (int64_t i = 0; i < max; i++) {
		int16_t* current_frame = test[i];
		for (int64_t j = 0; j < 512; j++) {
			file.write((char*)&current_frame[j], 2);
		}
	}
	file.close();

	//namespace speech = ::google::cloud::speech;
	//auto client = speech::speechclient(speech::makespeechconnection());

	//google::cloud::speech::v1::recognitionconfig config;
	//config.set_language_code("en-us");
	//google::cloud::speech::v1::recognitionaudio audio;
	//audio.set_content(str);
	//auto response = client.recognize(config, audio);
	//if (!response) throw std::runtime_error(response.status().message());
	//std::cout << response->debugstring() << "\n";

	
	return "asdf";
}

GoogleSpeechToTextService::~GoogleSpeechToTextService() {
	delete microphone_;
}