#pragma once
#include <string>
#include <map>

class ModuleRequest;

class IIntent {
public:
	virtual ~IIntent() {};
	virtual std::string GetModule() = 0;
	virtual std::string GetAction() = 0;
	virtual std::map<std::string, std::string> GetParameters() = 0;
	virtual std::string GetParameter(std::string) = 0;
	virtual std::string ToString() = 0;
	virtual int GetParametersSizer() = 0;

protected:

	std::string module_;
	std::string action_;
	int num_parameters_;
	std::map<std::string, std::string> parameters_;

};