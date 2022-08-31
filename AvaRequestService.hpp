#pragma once
#include "Logging.hpp"
#include <queue>
#include <condition_variable>
#include "ModuleRequest.hpp"

class TimeoutError : public std::exception {
public:
	const char* what() {
		return "timeout wating for requst";
	}
};

class AvaRequestService : private LoggerFactory{

public:
	AvaRequestService();


	void Notify(ModuleRequest& );
	
	ModuleRequest WaitForRequest();

private:
	std::priority_queue<ModuleRequest, std::vector<ModuleRequest>, ModuleRequestComparator> request_queue_;
	std::mutex mutex_;
	std::condition_variable condition_variable_;

};