#pragma once
#include <string>

class IIntent;

class ISpeechToIntentService {
public:
	virtual ~ISpeechToIntentService() {};
	virtual IIntent* GetIntent() = 0;
};