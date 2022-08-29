#pragma once
#include "IModuleService.hpp"
#include "Logging.hpp"
#include <Windows.h>
#include <list>

class ModuleService : public IModuleService, private LoggerFactory {
public:
	ModuleService();
	~ModuleService();

	int CountModules();
	void UnloadModules();
	void LoadModules();
private:
	void StartModule(std::string);

	HANDLE hjob_;
	std::list<PROCESS_INFORMATION> hproces_;
};
