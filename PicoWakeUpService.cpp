#pragma warning(disable : 4996)
#pragma warning(disable : 26812)


#include "PicoWakeUpService.hpp"
#include "ConfigurationManager.hpp"
#include <pv_recorder.h>
#include <pv_porcupine.h>
#include <cstring>
#include <iostream>
#include <cstdlib>

void PicoWakeUpService::SetupService() {
    LogInfo() << "Starting wake-up service...";
    if (is_setup_) {
        LogWarn() << "Wake-up service has alredy been set up.";
    }
    else {
        StartMic();
        StartPurcopine();
        is_setup_ = true;
        LogInfo() << "Wake-up service ready!";
    }
}

void PicoWakeUpService::StartMic() {
    LogDebug() << "Starting Mic...";

    const int32_t frame_length = pv_porcupine_frame_length();


    int32_t device_index = 0;

    LogDebug() << "Starting pv recorder.";
    pv_recorder_status_t recorder_status = pv_recorder_init(device_index, frame_length, 100, false, false, &recorder_);

    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Failed to initialize device with " << pv_recorder_status_to_string(recorder_status);
        exit(1);
    }

    const char* selected_device = pv_recorder_get_selected_device(recorder_);
    LogDebug() << "Selected device: " << selected_device;

    LogDebug() << "Start recording...";
    recorder_status = pv_recorder_start(recorder_);
    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Failed to start device with " << pv_recorder_status_to_string(recorder_status);
        exit(1);
    }

    pcm_ = static_cast<int16_t*>(malloc(frame_length * sizeof(int16_t)));
    if (!pcm_) {
        LogError() << "Failed to allocate pcm memory.";
        exit(1);
    }

    LogDebug() << "Mic ready!";
}

void PicoWakeUpService::StartPurcopine() {
    LogDebug() << "Starting purcopine...";
    LogDebug() << "Getting configuration variables...";
    static const char* acces_key = config.GetConfiguration("acces_key").c_str();
    const char* resources_path = config.GetConfiguration("resources_path").c_str();
    const char* model_file_path = config.GetConfiguration("model_file_path").c_str();
    const char* keyword_file_path = config.GetConfiguration("keyword_file_path").c_str();
    const float sensitivity = std::stof(config.GetConfiguration("sensitivity"));

    
    LogDebug() << "initiating purcopin.";
    const pv_status_t status = pv_porcupine_init(
        acces_key,
        model_file_path,
        1,
        &keyword_file_path,
        &sensitivity,
        &porcupine_);

    if (status != PV_STATUS_SUCCESS) {
        LogError() << "Failed to initiate porcupine." << pv_status_to_string(status);
    }

    LogDebug() << "Purcopine ready!";
}

void PicoWakeUpService::EndService() {
    LogInfo() << "Shuting down wake-up service...";
    if (!is_setup_) {
        LogWarn() << "Wake-up service has not been set up.";
    }
    else {
        StopMic();
        StopPurcopine();
        is_setup_ = false;
        LogInfo() << "Wake-up service has been shut down.";
    }
}

void PicoWakeUpService::StopMic() {
    LogDebug() << "Shutting down mic...";
    LogDebug() << "Stop recording...";
    pv_recorder_status_t recorder_status = pv_recorder_stop(recorder_);
    if (recorder_status != PV_RECORDER_STATUS_SUCCESS) {
        LogError() << "Failed to stop device with " << pv_recorder_status_to_string(recorder_status);
        exit(1);
    }

    LogDebug() << "Deleting pv_recorder...";
    pv_recorder_delete(recorder_);
    LogDebug() << "Freeing pcm...";
    free(pcm_);
    LogDebug() << "Mic has been shut down.";
}

void PicoWakeUpService::StopPurcopine() {
    LogDebug() << "Shutting down purcopine...";
    pv_porcupine_delete(porcupine_);
    LogDebug() << "Purcopine has been shut down.";
}

void PicoWakeUpService::WaitForWakeUp() {
    if (!is_setup_) {
        LogError() << "Wake-up service has not been set up!";
        exit(1);
    }
    LogInfo() << "Start processing audo. Listening for wake-up words.";
    volatile bool word_detected = false;
    while (!word_detected) {
        pv_recorder_read(recorder_, pcm_);
        int32_t keyword_index;
        const pv_status_t status = pv_porcupine_process(porcupine_, pcm_, &keyword_index);
        if (keyword_index != -1) {
            word_detected = true;
        }
    }
    LogInfo() << "Detected wake-up word!";
    
}

PicoWakeUpService::~PicoWakeUpService() {
    if (is_setup_) {
        EndService();
    }
}