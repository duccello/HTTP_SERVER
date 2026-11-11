#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#define READ_END 0
#define WRITE_END 1

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h> //for exit() DELETE!!!!
#include <iostream>

class Client;
class Request;
struct LocationConfig;

/*
	Initial version: doesn't have instances. 
	in the future: It could be change to create instances with states
	instead of static methods if needed eg. smarter clean-up
*/
class CgiHandler
{

	private:

		// OCF
		CgiHandler(); // to avoid empty cgiHandlers.
		CgiHandler(const CgiHandler&); //to avoid double close
		CgiHandler& operator=(const CgiHandler&);//to avoid double close

		// methods
		// the `env` will be created during the fork to be used in the execution (`exceve`)
		// there should be always the same env but it diff values per instance
		// static char** buildEnv(const Request& req,//method, query, content type
		// 						const std::string& fullPath,//script filename
		// 						const ServerConfig& server); // ports and timeout

		// static void freeEnv(char** env); // to free in the parent after the fork

		// static std::string getCgiBinary(const std::string& ext,// file extension
		// 						const LocationConfig& loc);//path
		static void	_closeChildFDs(void);
		static int	_childProcess(int in[2], int out[2], const std::string& fullPath, 
                               const LocationConfig& loc, char** env, char** args);
		static int	_setupCgiParent(int in[2], int out[2], Client& client, int pid);
		static void 		_buildCgiResponse(Client& client, std::string cgiOutput);
		
		//helpers
		static char** 	buildEnv(Client& client, const std::string& fullPath);
		static char**	buildArgs(const std::string& fullPath, const LocationConfig& loc);
		static void		cleanExecute(char **env, char **args);
		
	public:
		static int	pipeAndFork(Client& client, const std::string &fullPath, const LocationConfig& loc);
		static int	sendRequestToCgi(Client& client);
		static int	receiveCgiOutput(Client& client);
		static int  finishReadingCgi(Client& client);
		static void	removePipeFromEpoll(int fd, Client& client);
		static int	waitAndFinishCgi(Client& client);

};

#endif