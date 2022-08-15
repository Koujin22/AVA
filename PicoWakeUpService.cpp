#pragma warning(disable : 4996)
#pragma warning(disable : 26812)

#include "PicoWakeUpService.hpp"
#include "ConfigurationManager.hpp"
#include "PicoRecorderService.hpp"
#include <pv_porcupine.h>
#include <cstring>
#include <iostream>
#include <cstdlib>


PicoWakeUpService::PicoWakeUpService(std::shared_ptr<IMicrophoneService> microphone_service) : LoggerFactory(this), microhpone_{ microphone_service } {
    LogInfo() << "Starting wake-up service...";
    StartPurcopine();
    LogInfo() << "Wake-up service ready!";
};

void PicoWakeUpService::StartPurcopine() {
    LogDebug() << "Starting purcopine...";
    LogVerbose() << "Getting configuration variables...";
    static const char* acces_key = config.GetConfiguration("acces_key").c_str();
    const char* model_file_path = config.GetConfiguration("porcupine_model_file_path").c_str();
    const char* keyword_file_path = config.GetConfiguration("keyword_file_path").c_str();
    const float sensitivity = std::stof(config.GetConfiguration("porcupine_sensitivity"));

    LogVerbose() << "initiating purcopin.";
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

    const int32_t frame_length = pv_porcupine_frame_length();

    pcm_ = static_cast<int16_t*>(malloc(frame_length * sizeof(int16_t)));
    if (!pcm_) {
        LogError() << "Failed to allocate pcm memory.";
        exit(1);
    }

    LogDebug() << "Purcopine ready!";
}


void PicoWakeUpService::StopPurcopine() {
    LogDebug() << "Shutting down purcopine...";
    pv_porcupine_delete(porcupine_);

    LogVerbose() << "Freeing pcm...";
    free(pcm_);
    LogDebug() << "Purcopine has been shut down.";
}

void PicoWakeUpService::WaitForWakeUp() {
    LogInfo() << "Start processing audo. Listening for wake-up words.";
    volatile bool word_detected = false;
    while (!word_detected) {
        microhpone_->GetPcm(pcm_);
        int32_t keyword_index;
        const pv_status_t status = pv_porcupine_process(porcupine_, pcm_, &keyword_index);
        if (keyword_index != -1) {
            word_detected = true;
        }
    }
    LogInfo() << "Detected wake-up word!";
    
}

PicoWakeUpService::~PicoWakeUpService() {
    LogInfo() << "Shuting down wake-up service...";
    StopPurcopine();
    LogInfo() << "Wake-up service has been shut down.";
}