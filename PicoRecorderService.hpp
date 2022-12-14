#pragma once
#include "IMicrophoneService.hpp"
#include "Logging.hpp"

typedef struct pv_recorder pv_recorder_t;

class PicoRecorderService : public IMicrophoneService, private LoggerFactory {
public:
	PicoRecorderService();
	void GetPcm(int16_t*);
	void FlushPcm(int16_t*);
	~PicoRecorderService();
private:
	void StartMic();
	void StopMic();
	pv_recorder_t* recorder_;
};
