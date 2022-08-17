#pragma once
#include "ISpeechToText.hpp"
#include "PicoRecorderService.hpp"
#include "Logging.hpp"

class GoogleSpeechToTextService : public ITextToSpeechService, private LoggerFactory {

public:
	GoogleSpeechToTextService();
	std::string GetText(std::string);
	~GoogleSpeechToTextService();
private:
	IMicrophoneService* microphone_;
};