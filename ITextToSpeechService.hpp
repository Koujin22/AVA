#pragma once
#include <string>

class ITextToSpeechService {
public:
	virtual ~ITextToSpeechService() {};
	virtual void SayText(std::string) = 0;
	virtual void SaySsml(std::string) = 0;
protected:
	virtual std::string SsmlToString(std::string) = 0;
};
