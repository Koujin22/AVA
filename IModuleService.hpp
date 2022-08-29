#pragma once

class IModuleService {
public:
	virtual ~IModuleService() {};
	virtual void LoadModules() = 0;
	virtual void UnloadModules() = 0;
	virtual int CountModules() = 0;
};
