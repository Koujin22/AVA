#pragma once
#include "IWakeUpService.hpp"
#include "Logging.hpp"

typedef struct pv_porcupine pv_porcupine_t;
class IMicrophoneService;

class PicoWakeUpService : public IWakeUpService, private LoggerFactory {
public:
	PicoWakeUpService(std::shared_ptr<IMicrophoneService>);
	void WaitForWakeUp();
	~PicoWakeUpService();
private:
	void StartPurcopine();
	void StopPurcopine();

	int16_t* pcm_ = nullptr;
	std::shared_ptr<IMicrophoneService> microhpone_;
	pv_porcupine_t* porcupine_ = nullptr;
};