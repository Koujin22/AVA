
#include <Windows.h>
#include "ConfigurationManager.hpp"
#include "FrameworkManager.hpp"
#include "IIntent.hpp"

using namespace std;

namespace ConstStr {
    const string welcome = 
        R"(<speak>
            Ava 1.0 <break time="200ms"/>
            <emphasis level="strong">ready for action!</emphasis>
        </speak>)";
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    cout << "###############################################################################" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "||                                  A.V.A.                                   ||" << endl;
    cout << "||                                                                           ||" << endl;
    cout << "###############################################################################" << endl;

    LoggerFactory::SetLoggingLevel(INFO);
    config.LoadConfigurations("dev");

    FrameworkManager* framework_manager = new FrameworkManager();
    cout << "_______________________________________________________________________________" << endl;
    cout << "|                               Framework ready!                              |" << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    framework_manager->SaySsml(ConstStr::welcome, true);

    bool turn_off = false;
    bool understood = false;
    while (!turn_off) {

        framework_manager->ListenForWakeUpWord();

        framework_manager->SayText("What can I do for you sir?");
        understood = false;
        while (!understood) {
            std::unique_ptr<IIntent> intent(framework_manager->GetIntent());

            if (!intent) {
                framework_manager->SayText("Sorry, I couldn't get that. Could you repeat again, sir?");
            }
            else if (intent->GetModule() == "AVA" && intent->GetAction() == "turnoff") {
                understood = true;
                turn_off = true;
            }
            else if (intent->GetModule() == "TODO" && intent->GetAction() == "add") {
                understood = true;
                framework_manager->SayText("What do you want me to write down?");
                std::string dictation = framework_manager->GetText(10);

                framework_manager->SayText("Okay, is ");
                framework_manager->SayText(dictation, false, "es-us");
                framework_manager->SayText("correct ? ");

            }
            else {
                cout << "Intent understod yet it is not implemented yet." << endl;
                understood = true;
            }
            intent.reset();
        }
        framework_manager->SayText("Goodbye sir");
    }

    delete framework_manager;
    return 0;
}