
#include <Windows.h>
#include "ConfigurationManager.hpp"
#include "FrameworkManager.hpp"

using namespace std;

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    cout << "###############################################################################" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "||                                  A.V.A.                                   ||" << endl;
    cout << "||                                                                           ||" << endl;
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