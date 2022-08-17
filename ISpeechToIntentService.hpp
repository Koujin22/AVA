#pragma once

class IIntent;

class ISpeechToIntentService {
public:
	virtual ~ISpeechToIntentService() {};
	virtual IIntent* GetIntent() = 0;
};