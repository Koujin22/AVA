#pragma once
#include "IWakeUpService.hpp"
#include "Logging.hpp"

typedef struct pv_recorder pv_recorder_t; 
typedef struct pv_porcupine pv_porcupine_t;

class PicoWakeUpService : public IWakeUpService, private LoggerFactory {
public:
	PicoWakeUpService() : LoggerFactory(this) {};
	void WaitForWakeUp();
	void SetupService();
	void EndService();
	~PicoWakeUpService();
private:
	void StartMic();
	void StartPurcopine();
	void StopMic();
	void StopPurcopine();

	pv_recorder_t* recorder_ = nullptr;
	int16_t* pcm_ = nullptr;
	pv_porcupine_t* porcupine_ = nullptr;
};