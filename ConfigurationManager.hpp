#pragma once
#include "Logging.hpp"
#include <map>

class ConfigurationManager : private LoggerFactory {

public:

    ConfigurationManager() : LoggerFactory(this) {};
    void LoadConfigurations(std::string env);
    const std::string& GetConfiguration(std::string name);

private:

    std::map<std::string, std::string> config_values_;

};

extern ConfigurationManager config;