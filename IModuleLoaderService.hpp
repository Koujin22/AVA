#pragma once

class IModuleLoaderService {
public:
	virtual ~IModuleLoaderService() {};
	virtual void LoadModules() = 0;
	virtual void UnloadModules() = 0;
	virtual int CountModules() = 0;
};
