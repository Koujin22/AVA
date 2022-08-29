#include "PicoSpeechToIntentService.hpp"
#include "ConfigurationManager.hpp"
#include "IMicrophoneService.hpp"
#include "PicoIntent.hpp"
#include <pv_rhino.h>
#include <chrono>

PicoSpeechToIntentService::PicoSpeechToIntentService(std::shared_ptr<IMicrophoneService> microphone_service) : LoggerFactory(this), microhpone_{ microphone_service } {
	LogInfo() << "Starting speech-to-intent service...";
	StartRhino();
	LogInfo() << "Speech-to-intent serivce ready!";
}

void PicoSpeechToIntentService::StartRhino() {
    
    LogVerbose() << "Getting configuration variables...";
    static const char* acces_key = config.GetConfiguration("acces_key").c_str();
    const char* model_file_path = config.GetConfiguration("rhino_model_file_path").c_str();
    const char* context_path = config.GetConfiguration("context_file_path").c_str();
    const float sensitivity = std::stof(config.GetConfiguration("rhino_sensitivity"));
    const float wait_end = std::stof(config.GetConfiguration("rhino_silence"));
    const bool req_silence = std::stoi(config.GetConfiguration("rhino_silence"), NULL);

    LogVerbose() << "initiating rhino.";
    const pv_status_t status = pv_rhino_init(
        acces_key,
        model_file_path,
        context_path,
        sensitivity,
        wait_end,
        req_silence,
        &rhino_);

    if (status != PV_STATUS_SUCCESS) {
        LogError() << "Failed to initiate rhino." << pv_status_to_string(status);
    }

    const int32_t frame_length = pv_rhino_frame_length();

    LogDebug() << "Rhino frame length: " << frame_length;
    LogVerbose() << "Starting pcm malloc";
    pcm_ = static_cast<int16_t*>(malloc(frame_length * sizeof(int16_t)));
    if (!pcm_) {
        LogError() << "Failed to allocate pcm memory.";
        exit(1);
    }
}

void PicoSpeechToIntentService::StopRhino() {
    LogVerbose() << "Shutting down rhino...";
    pv_rhino_delete(rhino_);
    LogVerbose() << "Freeing pcm...";
    free(pcm_);
}

IIntent* PicoSpeechToIntentService::GetIntent() {

    LogInfo() << "Start speech-to-intent process.";


    bool is_finalized = false;

    while (!is_finalized) {
        is_finalized = ProcessFrame();
    }
    return Finalize();
}

IIntent* PicoSpeechToIntentService::GetConfirmation() {

    LogInfo() << "Start speech-to-intent process. Getting confirmation";


    bool is_finalized = false;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while (!is_finalized) {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) {
            LogDebug() << "Unable to understand confirmation.";
            pv_rhino_reset(rhino_);
            return nullptr;
        }
        is_finalized = ProcessFrame();
    }

    IIntent* intent = Finalize();

    if (intent->GetModule() == "AVA" && (intent->GetAction() == "yes" || intent->GetAction() == "no")) {
        return intent;
    }
    else {
        return nullptr;
    }
}

bool PicoSpeechToIntentService::ProcessFrame() {
    bool is_finalized = false;
    microhpone_->GetPcm(pcm_);
    pv_status_t status = pv_rhino_process(rhino_, pcm_, &is_finalized);
    if (status != PV_STATUS_SUCCESS) {
        LogError() << "Failed to process pcm frame. " << pv_status_to_string(status);
    }
    return is_finalized;
}

IIntent* PicoSpeechToIntentService::Finalize() {

    bool is_understood = false;

    pv_status_t status = pv_rhino_is_understood(rhino_, &is_understood);
    if (status != PV_STATUS_SUCCESS) {
        pv_rhino_reset(rhino_);
        LogError() << "Failed to check if understood. " << pv_status_to_string(status);
        return nullptr;
    }
    else if (!is_understood) {
        pv_rhino_reset(rhino_);
        LogDebug() << "Couldnt understand.";
        return nullptr;
    }


    IIntent* intent_ptr = nullptr;
    const char* intent = NULL;
    int32_t num_slots = 0;
    const char** slots = NULL;
    const char** values = NULL;

    status = pv_rhino_get_intent(rhino_, &intent, &num_slots, &slots, &values);
    if (status != PV_STATUS_SUCCESS) {
        LogError() << "Failed to get intent. " << pv_status_to_string(status);
        pv_rhino_reset(rhino_);
        return intent_ptr;
    } 
    if (slots != nullptr && values != nullptr) {
        LogInfo() << "Got intent. Intent: " << intent << " Slots: " << *slots << " Values: " << *values;
    }
    else {
        LogInfo() << "Got intent. Intent: " << intent;
    }
    intent_ptr = new PicoIntent(intent, num_slots, slots, values);

    pv_rhino_free_slots_and_values(rhino_, slots, values);
    pv_rhino_reset(rhino_);
    return intent_ptr;
}

PicoSpeechToIntentService::~PicoSpeechToIntentService() {
	LogInfo() << "Shuting down speech-to-intent service...";
	StopRhino();
	LogInfo() << "Speech-to-intent service has been shut down.";
}