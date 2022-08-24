
#include <Windows.h>
#include "ConfigurationManager.hpp"
#include "FrameworkManager.hpp"
#include <thread>

using namespace std;

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

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    EnableVTMode();
    std::this_thread::sleep_for(1000ms);

    cout << "###############################################################################" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "||                                  A.V.A.                                   ||" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "###############################################################################" << endl;
    cout << "||  TODO List:                                                               ||" << endl;
    cout << "||  -Poder poner sonido async.                                               ||" << endl;
    cout << "||  -Detectar comandos enviados por modulos                                  ||" << endl;
    cout << "||  -Limpiar codigo de modulos, mejorar loggin y limpiarlo.                  ||" << endl;
    cout << "###############################################################################" << endl;

    LoggerFactory::SetLoggingLevel(DEBUG);
    config.LoadConfigurations("dev");

    FrameworkManager* framework_manager = new FrameworkManager();
    cout << "_______________________________________________________________________________" << endl;
    cout << "|                               Framework ready!                              |" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    framework_manager->StartAvA();
    
    delete framework_manager;
    system("pause");
    return 0;
}