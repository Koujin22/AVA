#pragma once
#include "IIntent.hpp"

class PicoIntent : public IIntent {
public:
	PicoIntent(ModuleRequest& req);
	PicoIntent(const char* intent, int32_t num_slots, const char** slots, const char** values);
	std::string GetModule();
	std::string GetAction();
	std::map<std::string, std::string> GetParameters();
	std::string GetParameter(std::string);
	std::string ToString();
	int GetParametersSizer();
	~PicoIntent() {};
};