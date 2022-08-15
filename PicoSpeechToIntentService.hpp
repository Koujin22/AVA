#pragma once
#include "ISpeechToIntentService.hpp"
#include "Logging.hpp"
#include "IMicrophoneService.hpp"
#include <memory>


typedef struct pv_rhino pv_rhino_t;

class PicoSpeechToIntentService : public ISpeechToIntentService, private LoggerFactory {
public:
	PicoSpeechToIntentService(std::shared_ptr<IMicrophoneService>);
	IIntent* GetIntent();
	~PicoSpeechToIntentService();
private:
	void StartRhino();
	void StopRhino();

	int16_t* pcm_ = nullptr;
	std::shared_ptr<IMicrophoneService> microhpone_;
	pv_rhino_t* rhino_ = nullptr;
};