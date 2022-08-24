#include "ModuleService.hpp"
#include <string>
#include <thread>

ModuleService::ModuleService() : LoggerFactory(this) {

    //std::thread load_modules_thread([this] {LoadModules(); });
    std::string s = "AVA_Service";
    std::wstring nameJob = std::wstring(s.begin(), s.end());
    hjob_ = CreateJobObject(NULL, nameJob.c_str());
    LogWarn() << "Handle " << hjob_;

    JOBOBJECT_BASIC_LIMIT_INFORMATION  jobInfo;
    jobInfo.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    LogWarn() << "Set 16" << SetInformationJobObject(hjob_,
        JobObjectBasicLimitInformation, 
        &jobInfo,
        sizeof(jobInfo));

    LogWarn() << "Asign 21 " << AssignProcessToJobObject(hjob_, GetCurrentProcess());

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
    LogWarn() << "Asign 55 " << AssignProcessToJobObject(hjob_, pi.hProcess);
}; 
