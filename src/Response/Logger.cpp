#include "Logger.hpp"

#include <iostream>
#include <ctime>

std::string Logger::getTimestamp() const {
	std::time_t now = std::time(NULL);
	char buf[100];
	std::strftime(buf, sizeof(buf), 
				  "%Y/%m/%d %H:%M:%S", std::localtime(&now));//strftime() -> format time
	return std::string(buf);
}

Logger::Logger (const std::string& errorPath) {
	_error.open(errorPath.c_str(), std::ios::app); //std::ios::app is the append mode, it's a flag in <fstream>
	if (!_error)//when failes failbit is activated
		throw std::runtime_error("Logger failed to open files");

}

void Logger::closeLoggers(void)
{
    if (_error.is_open())
        _error.close();
}

Logger::~Logger (void)
{
	closeLoggers();
}

void Logger::logError(int statusCode, const std::string& path) {
	_error << "[" << getTimestamp() << "] ERROR " << statusCode << " "
		   << path << std::endl;
}
