#include "ConfigurationManager.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdlib.h>  
#include <vector>

using std::string;

void ConfigurationManager::LoadConfigurations(string env){

    string buffer;

    LogInfo() << "Loading config values";

    std::ifstream config_file("./resources/dev.properties");

    if (config_file.fail()) {
        LogError() << "Unable to find config file. Please provide one.";
        exit(1);
    }

    bool setting_env = false;

    while (std::getline(config_file, buffer)) {
        
        //we check if this line is ac commnent
        if (buffer.length() == 0 || buffer.at(0) == '#' || buffer.at(0) == ' ') {
            continue;
        }

        if (buffer.at(0) == '~') {
            setting_env = true;
            continue;
        }

        //we split token and value with = as identifier;
        size_t location = buffer.find("=");

        if (location == -1) {
            LogError() << "Invalid config file : Expected a <token >= <value> string in config file.";
            exit(1);
        }

        string token = buffer.substr(0, location);
        string value = buffer.substr(location + 1, buffer.length() - 1);
        
        if (!setting_env) {
            config_values_[token] = value;
        } else {
            int i = _putenv(buffer.c_str());
        }

    }

    config_file.close();

}

const string& ConfigurationManager::GetConfiguration(string name) {
    try {
        return config_values_.at(name);
    } catch (std::out_of_range ) {
        LogError() << "Could not find configuration with key: " << name << ". Please provide key value pair on file.";
        exit(1);
    }
}

string ConfigurationManager::GetConfigurationFromString(std::string name, int index) {
    if (index < 0) {
        LogError() << "Not a positive index.";
        exit(1);
    }
    string src = config_values_.at(name);
    int start = 0;
    int end = src.find(" ");
    for (int i = 0; i < index; i++) {
        src.erase(start, end + 1);
        end = src.find(" ");
    }
    return src.substr(start, end);
}

ConfigurationManager config;
