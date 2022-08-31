#include "PicoIntent.hpp"
#include "ModuleRequest.hpp"

using std::string;

PicoIntent::PicoIntent(ModuleRequest& req) {
	module_ = "MODULE_" + req.GetModule();
	action_ = "start_conversation";
}

PicoIntent::PicoIntent(const char* intent, int32_t num_slots, const char** slots, const char** values) {
	string s_intent(intent);
	size_t location = s_intent.find("_");
	module_ = s_intent.substr(0, location);
	action_ = s_intent.substr(location + 1, s_intent.length());

	for (int i = 0; i < num_slots; i++) {
		parameters_.emplace(slots[i], values[i]);
	}

}

string PicoIntent::GetModule() { return module_; }
string PicoIntent::GetAction() { return action_; }
string PicoIntent::GetParameter(string key) { return parameters_.at(key); }
std::map<string, string> PicoIntent::GetParameters() { return parameters_; }
int PicoIntent::GetParametersSizer() { return num_parameters_; }


string PicoIntent::ToString() {
	if (parameters_.size() > 0) {
		std::string slots = "";
		for (const auto& [k, v] : parameters_) {
			slots.append(k);
			slots.append(":");
			slots.append(v);
			slots.append(",");
		}
		return module_ + " " + action_ + " " + slots;
	}
	else {
		return module_ + " " + action_ + " ";
	}
}
