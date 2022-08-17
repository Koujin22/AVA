#pragma once
#include <string>

class ISpeechToTextService {
public:
	virtual ~ISpeechToTextService() {};
	virtual std::string GetText(int) = 0;
};
