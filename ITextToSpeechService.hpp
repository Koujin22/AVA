#pragma once
#include <string>
#include "IFrameworkService.hpp"

class ITextToSpeechService : public IFrameworkService{
public:
	virtual ~ITextToSpeechService() {};
	virtual void Say(std::string msg) = 0;
};
