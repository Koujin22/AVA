#pragma once
#include "Logging.hpp"
#include "ISpeechToIntentService.hpp"

typedef struct pv_rhino pv_rhino_t;
class IMicrophoneService;

class PicoSpeechToIntentService : public ISpeechToIntentService, private LoggerFactory {
public:
	PicoSpeechToIntentService(std::shared_ptr<IMicrophoneService>);
	IIntent* GetIntent();
	IIntent* GetConfirmation();
	~PicoSpeechToIntentService();
private:
	void StartRhino();
	void StopRhino();

	bool ProcessFrame();
	IIntent* Finalize();

	int16_t* pcm_ = nullptr;
	std::shared_ptr<IMicrophoneService> microhpone_;
	pv_rhino_t* rhino_ = nullptr;
};