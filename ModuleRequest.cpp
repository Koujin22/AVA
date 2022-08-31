#include "ModuleRequest.hpp"

using std::string;


ModuleRequest::ModuleRequest(std::string& msg, int end_module, int end_req) :
	module_{ msg.data(), end_module },
	priority_{ std::stoi(msg.substr(end_req + 1)) } { }

const string& ModuleRequest::GetModule() { return module_; }

const int& ModuleRequest::GetPriority() { return priority_; }

bool ModuleRequestComparator::operator() (ModuleRequest req1, ModuleRequest req2) {
	if (req1.GetPriority() < req2.GetPriority()) return true;
	else return false;
}

string ModuleRequest::ToString() {
	return "Module: " + module_ + " priority: " + std::to_string(priority_);
}