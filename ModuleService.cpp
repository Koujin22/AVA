#include "ModuleService.hpp"
#include <string>
#include <thread>
#include <strsafe.h>

ModuleService::ModuleService() : LoggerFactory(this) {

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
        LogWarn() << "Process is already in Job";
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

ModuleService::~ModuleService() {};

void ModuleService::LoadModules() {


    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::string t = "C:\\Windows\\system32\\cmd.exe /k Mods\\.venv\\Scripts\\python.exe Mods/note_taker/main.py";
    std::string c = "C:\\Windows\\system32\\cmd.exe";

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    LogInfo() << "Creating process";
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
    LogWarn() << "Values of handles: hjob " << hjob_ << " piprocess " << pi.hProcess;
    LogWarn() << "Asign 55 " << AssignProcessToJobObject(hjob_, pi.hProcess);
}; 
