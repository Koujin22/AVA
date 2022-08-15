#pragma once
#pragma warning(disable : 26812)
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

enum LOG_LEVELS {
	NONE = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4,
	VERBOSE = 5, 

};


class LoggerFactory {

public:

	template<typename T>
	LoggerFactory(T t) : class_name{ getFormatedClassName(typeid(*t).name())} {};

	static void SetLoggingLevel(LOG_LEVELS newLevel) {
		level = newLevel;
		std::string string_new_level;
		switch (newLevel) {
		case 3:
			string_new_level = "INFO";
			break;
		case 4:
			string_new_level = "DEBUG";
			break;
		case 5:
			string_new_level = "VERBOSE";
			break;

		}
		if (newLevel >= 3) {
			std::cout << "|   INFO|               Logger | Setting log level to " << string_new_level << std::endl;
		}
	}

protected:
	
	class Logger {
	public:
		Logger(std::string name, std::string level, bool will_print) : will_print{ will_print } {
			s << "|" << std::setw(7) << level << "| " << std::setw(20) << name << " | ";
		};

		template <class T>
		Logger& operator<<(const T& x) {
			s << x ;
			return *this;
		}

		~Logger() {
			if (will_print) {
				std::cout << s.rdbuf() << std::endl;
			}			
		}

	private:
		std::stringstream s;
		bool will_print;

	};

	Logger LogError() { return Log("ERROR", level >= 1); }
	Logger LogWarn() { return Log("WARN", level >= 2); }
	Logger LogInfo() { return Log("INFO", level >= 3); }
	Logger LogDebug() { return Log("DEBUG", level >= 4); }
	Logger LogVerbose() { return Log("VERBOSE", level >= 5); }

	Logger Log(std::string level, bool will_print) {
		return Logger(class_name, level, will_print);
	}

	inline static volatile LOG_LEVELS level = LOG_LEVELS::ERROR;

private:

	static std::string getFormatedClassName(std::string unformated) {
		return unformated.substr(unformated.find(" ")+1, unformated.length() - 1);
	};
	std::string class_name;
};

