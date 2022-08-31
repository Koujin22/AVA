#pragma once
#include <string>

class ModuleRequest {
public:
	ModuleRequest();
	ModuleRequest(std::string&, int end_module, int end_req);
	const std::string& GetModule();
	const int& GetPriority();
	std::string ToString();
	bool IsValid();
private:
	std::string module_;
	int priority_;
	bool is_valid_;
};

class ModuleRequestComparator {
public:
	bool operator() (ModuleRequest, ModuleRequest);
};