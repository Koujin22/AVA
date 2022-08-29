#pragma once
#include <string>

class ModuleCommand {
public:
	ModuleCommand(std::string_view);
	const std::string& GetCommand();
	const std::string& GetParam();
	const std::string& GetVars();
	bool IsAsync();
	int GetTime();
	std::string GetLang();
	std::string ToString();
private:
	ModuleCommand(std::string_view, int end_command);
	std::string command_;
	std::string param_;
	std::string vars_;
	bool async_;
	std::string lang_;


};