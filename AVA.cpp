
#include <Windows.h>
#include "ConfigurationManager.hpp"
#include "PicoWakeUpService.hpp"
#include "PicoRecorderService.hpp"
#include "AvaProcess.hpp"
#include "PicoIntent.hpp"
#include "AvaUserInteraction.hpp"
#include "Ava.hpp"
#include <thread>
#include <zmq.hpp>
#include "AvaModuleRequest.hpp"
#include "AvaRequestService.hpp"
#include "AvaModuleInteraction.hpp"


AVA::AVA() : AVA(std::make_shared<PicoRecorderService>(), std::make_shared<zmq::context_t>()) {}

AVA::AVA(std::shared_ptr<IMicrophoneService> microphone, std::shared_ptr<zmq::context_t> context) : AVA(std::make_shared<AvaProcess>(microphone, context), microphone, context, std::make_shared<AvaRequestService>()) {}

AVA::AVA(std::shared_ptr<AvaProcess> ava, std::shared_ptr<IMicrophoneService> microphone, std::shared_ptr<zmq::context_t> context, std::shared_ptr<AvaRequestService> req) :
    LoggerFactory(this),
    ava_user_interaction_{new AvaUserInteraction(ava, microphone)},
    ava_module_request_{new AvaModuleRequest (context, req) },
    ava_module_interaction_{new AvaModuleInteraction(ava, req)}
{}

void AVA::Start() {
    std::thread ava_user_thread(&AvaUserInteraction::Start, ava_user_interaction_);
    std::thread ava_module_request_thread(&AvaModuleRequest::Start, ava_module_request_);
    std::thread ava_module_interaction_thread(&AvaModuleInteraction::Start, ava_module_interaction_);

    ava_user_thread.join();
    ava_module_request_->Stop();
    ava_module_request_thread.join();
    ava_module_interaction_->Stop();
    ava_module_interaction_thread.join();
}


AVA::~AVA() {
    delete ava_user_interaction_;
    delete ava_module_request_;
}

bool EnableVTMode()
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return false;
    }
    return true;
}


std::mutex LoggerFactory::mutex_;


/*/

Log Levels

## LogError : Use when it is a fatal error. It must be before exit.
## LogWarn : Use when something failed but its not enough to stop execution.
## LogInfo : Mayor steps or changes in stage the program is taking. 
## LogDebug : Use to dump information regarding execution. Not for static things.
## LogVerbose : Use to detail how the LogInfo before is doing stuff. 

*/

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    EnableVTMode();

   
    config.LoadConfigurations("dev");
    LoggerFactory::SetLoggingLevel(DEBUG);

    AVA a;

    a.Start();
    
    return 0;
}