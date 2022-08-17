#pragma once
#include <string>

class ITextToSpeechService {
public:
	virtual ~ITextToSpeechService() {};
	virtual std::string GetText(std::string) = 0;
};
