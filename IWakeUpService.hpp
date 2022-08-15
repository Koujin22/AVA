#pragma once
#include "IFrameworkService.hpp"

class IWakeUpService : public IFrameworkService {
public:
	virtual ~IWakeUpService() {};
	virtual void WaitForWakeUp() = 0;
};