#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <string>


class Client;

class Logger {
	private:
		std::ofstream _error;

		Logger(); //to prevent creating objects without param

		Logger(const Logger&);// to avoid std::ofstream copies (2 loggers writing in the same fd)

		Logger& operator=(const Logger&);
		
		//private methods
		std::string getTimestamp() const;

	public:
		Logger (const std::string& errorPath);
		~Logger (void);

		void logError(int statusCode, const std::string& path);
		void closeLoggers(void);
};

#endif