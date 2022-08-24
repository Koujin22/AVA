#pragma once
#include "IModuleService.hpp"
#include "Logging.hpp"
#include <Windows.h>
#include <list>

class ModuleService : public IModuleService, private LoggerFactory {
public:
	ModuleService();
	~ModuleService();
	void LoadModules();
private:
	HANDLE hjob_;
	std::list<HANDLE> hproces_;
};
