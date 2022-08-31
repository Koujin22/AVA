#pragma once
#include "Logging.hpp"
#include "ISpeechToIntentService.hpp"

typedef struct pv_rhino pv_rhino_t;
class IMicrophoneService;

class PicoSpeechToIntentService : public ISpeechToIntentService, private LoggerFactory {
public:
	PicoSpeechToIntentService(std::shared_ptr<IMicrophoneService>);
	std::unique_ptr<IIntent> GetIntent();
	std::unique_ptr<IIntent> GetConfirmation();
	~PicoSpeechToIntentService();
private:
	void StartRhino();
	void StopRhino();

	bool ProcessFrame();
	std::unique_ptr<IIntent> Finalize();

	int16_t* pcm_ = nullptr;
	std::shared_ptr<IMicrophoneService> microhpone_;
	pv_rhino_t* rhino_ = nullptr;
};