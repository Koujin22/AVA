
#include <Windows.h>
#include "ConfigurationManager.hpp"
#include "FrameworkManager.hpp"

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

    cout << "###############################################################################" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "||                                  A.V.A.                                   ||" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "###############################################################################" << endl;
    cout << "||  TODO List:                                                               ||" << endl;
    cout << "||  -Hacer que el modulo note_taker haga su funcion                          ||" << endl;
    cout << "||  -Corra ava sin ventana y al inciar compu                                 ||" << endl;
    cout << "||  -logger thread-safe2                                                     ||" << endl;
    cout << "||  -refactorizar framework manager startava                                 ||" << endl;
    cout << "###############################################################################" << endl;

    LoggerFactory::SetLoggingLevel(DEBUG);
    config.LoadConfigurations("dev");

    FrameworkManager* framework_manager = new FrameworkManager();
    cout << "_______________________________________________________________________________" << endl;
    cout << "|                               Framework ready!                              |" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    framework_manager->StartAvA();
    
    delete framework_manager;
    return 0;
}