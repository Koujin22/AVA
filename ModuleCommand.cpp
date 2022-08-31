#include "ModuleCommand.hpp"

using std::string;

ModuleCommand::ModuleCommand(std::string_view msg) :
	ModuleCommand(msg, msg.find("_")) {}

ModuleCommand::ModuleCommand(std::string_view msg_view, int end_command) :
	LoggerFactory(this),
	command_{ msg_view.data(), end_command },
	param_{msg_view.data() +end_command+1, msg_view.find("#") - end_command - 1 },
	vars_{msg_view.data()+ msg_view.find("#") + 1, msg_view.size() - msg_view.find("#") - 1} {
}

const string& ModuleCommand::GetCommand() { return command_; }
const string& ModuleCommand::GetParam() { return param_; }
const string& ModuleCommand::GetVars() { return vars_; }
bool ModuleCommand::IsAsync() {
	if (vars_.find('A') == -1) {
		return false;
	}
	return vars_.at(vars_.find('A') + 1) == 't';
}
string ModuleCommand::GetLang() {
	if (vars_.find('L') == -1) {
		return "en-us";
	}
	return  vars_.substr(vars_.find('L') + 1, vars_.find('L') + 3);
}

int ModuleCommand::GetTime() {
	if (vars_.find('T') == -1) {
		return 5;
	}
	int duration = std::stoi(vars_.substr(vars_.find('T') + 1, vars_.find('T') + 2));
	if (duration > 5) {
		LogWarn() << "Detected more than 10 seconds. Need a safeway for modules to request longer things.";
	}
	return (duration > 5) ? 5 : duration;
}

string ModuleCommand::ToString() {
	return "Command: " + command_ + " Vars: " + vars_ + "Param: " + param_;
}