#include "PicoRecorderService.hpp"
#include <pv_recorder.h>

PicoRecorderService::PicoRecorderService() : LoggerFactory(this) {
    LogInfo() << "Starting microphone service...";
    StartMic();
    LogInfo() << "Microphone service ready!";
};

void PicoRecorderService::StartMic() {

    LogVerbose() << "Starting pv recorder.";

    int32_t device_index = 0;

    const int32_t frame_length = 512;

    pv_recorder_status_t recorder_status = pv_recorder_init(device_index, frame_length, 100, false, false, &recorder_);


    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Failed to initialize device with " << pv_recorder_status_to_string(recorder_status);
        exit(1);
    }

    const char* selected_device = pv_recorder_get_selected_device(recorder_);
    LogDebug() << "Selected device: " << selected_device;

    LogVerbose() << "Start recording...";
    recorder_status = pv_recorder_start(recorder_);
    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Failed to start device with " << pv_recorder_status_to_string(recorder_status);
        exit(1);
    }

}

void PicoRecorderService::StopMic() {
    LogDebug() << "Shutting down mic...";
    pv_recorder_status_t recorder_status = pv_recorder_stop(recorder_);
    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Failed to stop device with " << pv_recorder_status_to_string(recorder_status);
        exit(1);
    }

    LogVerbose() << "Deleting pv_recorder...";
    pv_recorder_delete(recorder_);
}

void PicoRecorderService::GetPcm(int16_t* pcm) {

    pv_recorder_status_t recorder_status = pv_recorder_read(recorder_, pcm);
    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Error while trying to read microhpone. " << pv_recorder_status_to_string(recorder_status);
    }
}

void PicoRecorderService::FlushPcm(int16_t* pcm) {
    for (int i = 0; i < 10; i++) {
        GetPcm(pcm);
    }
}

PicoRecorderService::~PicoRecorderService() {
    LogInfo() << "Shuting down microphone service...";
    StopMic();
    LogInfo() << "Microphone service has been shut down.";
}