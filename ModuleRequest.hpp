#pragma once
#include <string>
class ModuleRequest {
public:
	ModuleRequest(std::string&, int end_module, int end_req);
	const std::string& GetModule();
	const int& GetPriority();
	std::string ToString();
private:
	std::string module_;
	int priority_;
};

class ModuleRequestComparator {
public:
	bool operator() (ModuleRequest, ModuleRequest);
};