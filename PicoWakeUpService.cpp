#pragma warning(disable : 4996)
#pragma warning(disable : 26812)

#include "PicoWakeUpService.hpp"
#include "ConfigurationManager.hpp"
#include "IMicrophoneService.hpp"
#include <pv_porcupine.h>


PicoWakeUpService::PicoWakeUpService(std::shared_ptr<IMicrophoneService> microphone_service) : LoggerFactory(this), microhpone_{ microphone_service } {
    LogInfo() << "Starting wake-up service...";
    StartPurcopine();
    LogInfo() << "Wake-up service ready!";
};

void PicoWakeUpService::StartPurcopine() {
    LogVerbose() << "Getting configuration variables...";
    const int num_keyword_files = std::stoi(config.GetConfiguration("num_keyword_files"));
    static const char* acces_key = config.GetConfiguration("acces_key").c_str();
    //delete on test_tmp, test y sensitivity;
    const char* model_file_path = config.GetConfiguration("porcupine_model_file_path").c_str();
    std::string* test_tmp = new std::string[num_keyword_files];
    const char** test = new const char* [num_keyword_files];
    float* sensitivity = new float[num_keyword_files];
    for (int i = 0; i < num_keyword_files; i++) {
        test_tmp[i] = config.GetConfigurationFromString("keyword_file_path", i);
        test[i] = test_tmp[i].c_str();
        sensitivity[i] = std::stof(config.GetConfiguration("porcupine_sensitivity"));
    }

    LogVerbose() << "initiating purcopin.";
    const pv_status_t status = pv_porcupine_init(
        acces_key,
        model_file_path,
        num_keyword_files,
        test,
        sensitivity,
        &porcupine_);

    if (status != PV_STATUS_SUCCESS) {
        LogError() << "Failed to initiate porcupine." << pv_status_to_string(status);
    }

    const int32_t frame_length = pv_porcupine_frame_length();

    LogDebug() << "Framelength from porcupine: " << frame_length;
    LogVerbose() << "Starting pcm malloc.";

    pcm_ = static_cast<int16_t*>(malloc(frame_length * sizeof(int16_t)));
    if (!pcm_) {
        LogError() << "Failed to allocate pcm memory.";
        exit(1);
    }

    LogVerbose() << "Deleting temporal array containers.";

    delete[] test_tmp;
    delete[] test;
    delete[] sensitivity;
}

void PicoWakeUpService::StopPurcopine() {
    LogVerbose() << "Shutting down purcopine...";
    pv_porcupine_delete(porcupine_);

    LogVerbose() << "Freeing pcm...";
    free(pcm_);
}

void PicoWakeUpService::WaitForWakeUp() {
    LogInfo() << "Start processing audo. Listening for wake-up words.";
    
    Flush();
    
    volatile bool word_detected = false;
    int32_t keyword_index;
    while (!word_detected) {
        microhpone_->GetPcm(pcm_);
        const pv_status_t status = pv_porcupine_process(porcupine_, pcm_, &keyword_index);
        if (keyword_index != -1) {
            word_detected = true;
        }
    }
    
    LogInfo() << "Detected wake-up word! Index: "<< keyword_index;
    
}

void PicoWakeUpService::Flush() {
    int32_t dump;
    for (int i = 0; i < 40; i++) {
        microhpone_->GetPcm(pcm_);
        pv_porcupine_process(porcupine_, pcm_, &dump);
    }
}

PicoWakeUpService::~PicoWakeUpService() {
    LogInfo() << "Shuting down wake-up service...";
    StopPurcopine();
    LogInfo() << "Wake-up service has been shut down.";
}