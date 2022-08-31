#include "AvaUserInteraction.hpp"
#include "PicoWakeUpService.hpp"
#include "AvaProcess.hpp"
#include "AvaRequestService.hpp"
#include "AvaCommandResult.hpp"

namespace ConstStr {
    const std::string welcome =
        R"(<speak>
            Ava 1.0 <break time="200ms"/>
            <emphasis level="strong">ready for action!</emphasis>
        </speak>)";
    const std::string bye =
        R"(<speak>
            Goodbye sir!
        </speak>)";
}

AvaUserInteraction::AvaUserInteraction(std::shared_ptr<AvaProcess> ava, std::shared_ptr<IMicrophoneService> microphone, std::shared_ptr<AvaRequestService> ava_req) :
    LoggerFactory(this),
    wake_up_service_{ new PicoWakeUpService(microphone) },
    ava_req_{ ava_req },
    ava_{ ava } { }

void AvaUserInteraction::Start() {
    ava_->SaySsml(ConstStr::welcome, true);
    Run();
}

void AvaUserInteraction::Run() {
    bool turn_off = false;
    while (!turn_off) {

        ListenForWakeUpWord();
        std::unique_ptr<AvaCommandResult> result = ava_->Run();
        if (result) {
            turn_off = result->IsOff();
        }
        ava_req_->Notify();
    }
    ava_->SaySsml(ConstStr::bye);
}

void AvaUserInteraction::ListenForWakeUpWord() {
    wake_up_service_->WaitForWakeUp();
}

AvaUserInteraction::~AvaUserInteraction() {
    delete wake_up_service_;
}
