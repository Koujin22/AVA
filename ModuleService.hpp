#pragma once
#include "IModuleService.hpp"
#include "Logging.hpp"

class ModuleService : public IModuleService, private LoggerFactory {
public:
	ModuleService();
	~ModuleService();
	void LoadModules();
};
