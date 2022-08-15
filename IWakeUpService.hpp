#pragma once

class IWakeUpService {
public:
	virtual ~IWakeUpService() {};
	virtual void WaitForWakeUp() = 0;
};