#include "ModuleService.hpp"
#include <string>
#include <thread>
#include <strsafe.h>

ModuleService::ModuleService() : LoggerFactory(this) {
    LogVerbose() << "Creating and configuring Job for processess.";
    //std::thread load_modules_thread([this] {LoadModules(); });
    std::string s = "AVA_Service";
    std::wstring nameJob = std::wstring(s.begin(), s.end());
    BOOL bIsProcessInJob;
    BOOL bSuccess = IsProcessInJob(GetCurrentProcess(), NULL, &bIsProcessInJob);
    if (bSuccess == 0) {
        LogError() << "IsProcessInJob failed: error " << GetLastError();
        exit(1);
        
    }
    if (bIsProcessInJob) {
        LogDebug() << "Process is already in Job";
    }
    hjob_ = CreateJobObject(NULL, nameJob.c_str());

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = { 0 };
    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    bSuccess = SetInformationJobObject(hjob_,
        JobObjectExtendedLimitInformation, 
        &jobInfo,
        sizeof(jobInfo));
    if (bSuccess == 0) {
        LogError() << "SetInformationJobObject failed: error ";
        exit(1);
    }

};

ModuleService::~ModuleService() {
    LogInfo() << "Shutting down modules...";
    std::list<HANDLE>::iterator it;
    for (it = hproces_.begin(); it != hproces_.end(); it++)
    {
        CloseHandle(*it);
    }

    CloseHandle(hjob_);
    LogInfo() << "Modules service has been shut down.";
};

void ModuleService::LoadModules() {

    LogDebug() << "Starting module " << "note_taker";// note_taker debe ser dinamico. 
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::string t = "C:\\Windows\\system32\\cmd.exe /k Mods\\.venv\\Scripts\\python.exe -m Mods.note_taker.main";
    std::string c = "C:\\Windows\\system32\\cmd.exe";

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
        const_cast<char*>(c.c_str()),   // No module name (use command line)
        const_cast<char*>(t.c_str()),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        LogError() << "CreateProcess failed (%d)." << GetLastError();

    }
    AssignProcessToJobObject(hjob_, pi.hProcess);
    hproces_.push_back(pi.hProcess);
    hproces_.push_back(pi.hThread);
}; 
