#include "ModuleLoaderService.hpp"
#include "WindowsUtil.hpp"
#include <string>
#include <thread>
#include <WinUser.h>
#include <strsafe.h>
#include <filesystem>
#include <iostream>



ModuleLoaderService::ModuleLoaderService() : LoggerFactory(this) {
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

ModuleLoaderService::~ModuleLoaderService() {
    LogInfo() << "Shutting down modules...";
    UnloadModules();
    CloseHandle(hjob_);
    LogInfo() << "Modules service has been shut down.";
};


void ModuleLoaderService::UnloadModules() {
    std::list<PROCESS_INFORMATION>::iterator it;
    for (it = hproces_.begin(); it != hproces_.end(); it++)
    {
        WaitForInputIdle(it->hProcess, INFINITE);

        if (WaitForSingleObject(it->hProcess, 1000) == WAIT_TIMEOUT)
        {
            EnumWindows(&win::SendWMCloseMsg, it->dwProcessId);
            if (WaitForSingleObject(it->hProcess, 1000) == WAIT_TIMEOUT)
            {
                // application did not close in a timely manner, do something...

                // in this example, just kill it.  In a real world
                // app, you should ask the user what to do...
                LogWarn() << "Terminated process";
                TerminateProcess(it->hProcess, 0);
            }
        }

        CloseHandle(it->hProcess);
    }

    hproces_.clear();
}


int ModuleLoaderService::CountModules() {
    int count = 0;
    for (auto& p : std::filesystem::directory_iterator("Mods"))
        if (p.is_directory() && p.path().filename().string().at(0) != '.' && p.path().filename().string().at(0) != '_') {
            count++;
        }
    return count;
}

void ModuleLoaderService::LoadModules() {

    std::vector<std::string> r;
    for (auto& p : std::filesystem::directory_iterator("Mods"))
        if (p.is_directory() && p.path().filename().string().at(0) != '.' && p.path().filename().string().at(0) != '_') {
            r.push_back(p.path().filename().string());
            LogDebug() << "Module: " << p.path().filename().string() << " detected";
        }

    
    for (std::string module : r) {
        StartModule(module);
    }
}; 


void ModuleLoaderService::StartModule(std::string module_name) {
    LogDebug() << "Starting module " << module_name;// note_taker debe ser dinamico. 
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    std::string t = "C:\\Windows\\system32\\cmd.exe /k Mods\\.venv\\Scripts\\python.exe -m Mods."+ module_name +".main";
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
    hproces_.push_back(pi);
    CloseHandle(pi.hThread);
}