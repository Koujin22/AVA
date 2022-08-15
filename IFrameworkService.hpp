#pragma once

class IFrameworkService {
public:
	virtual ~IFrameworkService() {};
	virtual void SetupService() = 0;
	virtual void EndService() = 0;
	virtual bool HasBeenSetUp() {
		return is_setup_;
	}
protected:
	bool is_setup_ = false;

};