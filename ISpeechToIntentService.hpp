#pragma once

class IIntent;

class ISpeechToIntentService {
public:
	virtual ~ISpeechToIntentService() {};
	virtual std::unique_ptr<IIntent> GetIntent() = 0;
	virtual std::unique_ptr<IIntent> GetConfirmation() = 0;
};