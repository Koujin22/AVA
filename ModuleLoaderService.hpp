#pragma once
#include "IModuleLoaderService.hpp"
#include "Logging.hpp"
#include <Windows.h>
#include <list>

class ModuleLoaderService : public IModuleLoaderService, private LoggerFactory {
public:
	ModuleLoaderService();
	~ModuleLoaderService();

	int CountModules();
	void UnloadModules();
	void LoadModules();
private:
	void StartModule(std::string);

	HANDLE hjob_;
	std::list<PROCESS_INFORMATION> hproces_;
};
