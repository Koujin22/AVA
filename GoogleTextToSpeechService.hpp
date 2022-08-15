#pragma once
#include "ITextToSpeechService.hpp"
#include "Logging.hpp"

class GoogleTextToSpeechService : public ITextToSpeechService, private LoggerFactory {
public:
	GoogleTextToSpeechService() : LoggerFactory(this) {};
	void Say(std::string msg);
	void SetupService();
	void EndService();
	~GoogleTextToSpeechService();
private:


};
