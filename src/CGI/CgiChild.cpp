#include "CgiHandler.hpp"
#include "Webserv.hpp"
#include <unistd.h>

int CgiHandler::_childProcess(int in[2], int out[2], const std::string& fullPath, 
                               const LocationConfig& loc, char** env, char** args)
{
    close(in[WRITE_END]);
    close(out[READ_END]);
	if (dup2(in[READ_END], STDIN_FILENO) == -1)
	{
		_closeChildFDs();
		close(in[READ_END]);
		close(out[WRITE_END]);
		cleanExecute(env, args);
		std::cerr << "Duping error\n";
		return (1);
	}
	close(in[READ_END]);
	if (dup2(out[WRITE_END], STDOUT_FILENO) == -1)
	{
		_closeChildFDs();
		close(out[WRITE_END]);
		cleanExecute(env, args);
		std::cerr << "Duping error\n";
		return (1);
	}
	close(out[WRITE_END]);
	_closeChildFDs();
	for (std::map<std::string, std::string>::const_iterator handlersIt = loc.cgiHandlers.begin();
		handlersIt != loc.cgiHandlers.end();
		++handlersIt)
	{
        if (fullPath.find(handlersIt->first) != std::string::npos)
		{
			std::string scriptDir = fullPath.substr(0, fullPath.find_last_of('/'));
			if (chdir(scriptDir.c_str()) == -1)
			{
				return (1);
			}
			execve(handlersIt->second.c_str(), args, env);
		}
	}
	cleanExecute(env, args);
	std::cerr << "Couldn't execute CGI\n";
	return (1);
}

void CgiHandler::_closeChildFDs(void)
{
    Webserv& webserv = Webserv::getInstance();
    webserv.closeSockets();
    webserv.getLogger().closeLoggers();
}
