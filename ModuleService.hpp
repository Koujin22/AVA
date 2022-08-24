#pragma once
#include "IModuleService.hpp"
#include "Logging.hpp"
#include <Windows.h>

class ModuleService : public IModuleService, private LoggerFactory {
public:
	ModuleService();
	~ModuleService();
	void LoadModules();
private:
	HANDLE hjob_;
};
