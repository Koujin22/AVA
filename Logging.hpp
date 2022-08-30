#pragma once
#pragma warning(disable : 26812)
#include <iomanip>
#include <sstream>
#include <iostream>


enum LOG_LEVELS {
	NONE = 0,
	ERR = 1,
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
			std::cout << "|"<< GetColorSequence(COLORS::GREEN) << "   INFO" << "\x1B[0m" << "|                 Logger | Setting log level to " << string_new_level << std::endl;
		}
	}

protected:
	enum class COLORS {
		RED,
		GREEN,
		DEFAULT,
		BLUE,
		YELLOW
	};

	class Logger {
	public:
		Logger(std::string name, std::string level, bool will_print, std::string color) : will_print{will_print} {
			s << "|" << color << std::setw(7) << level << "\x1B[0m" << "| " << std::setw(22) << name << " | ";
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

	Logger LogError() { return Log("ERROR", level >= 1, COLORS::RED);  }
	Logger LogWarn() { return Log("WARN", level >= 2, COLORS::YELLOW); }
	Logger LogInfo() { return Log("INFO", level >= 3, COLORS::GREEN); }
	Logger LogDebug() { return Log("DEBUG", level >= 4, COLORS::BLUE); }
	Logger LogVerbose() { return Log("VERBOSE", level >= 5); }

	Logger Log(std::string level, bool will_print, COLORS color = COLORS::DEFAULT) {
		return Logger(class_name, level, will_print, GetColorSequence(color));
	}

	inline static volatile LOG_LEVELS level = LOG_LEVELS::ERR;

private:

	
	static std::string GetColorSequence(COLORS fg_color) {
		switch (fg_color) {
		case COLORS::RED:
			return "\x1B[31m";
		case COLORS::YELLOW:
			return "\x1B[33m";
		case COLORS::GREEN:
			return "\x1B[32m";
		case COLORS::BLUE:
			return "\x1B[34m";
		case COLORS::DEFAULT:
			return "\x1B[0m";
		default:
			return "";
		}
	}

	static std::string getFormatedClassName(std::string unformated) {
		return unformated.substr(unformated.find(" ")+1, unformated.length() - 1);
	};
	std::string class_name;
};

