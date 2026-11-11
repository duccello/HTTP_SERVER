#ifndef WEBSERV_HPP
#define WEBSERV_HPP


#include "Listener.hpp"
#include "Logger.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "Router.hpp"
#include "Config_Structs.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h> // for "close()" ALTERNATIVES???
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <csignal>
#include <stdexcept>

class Client;

struct FdInfo
{
	FdInfo(int pipefd, std::string to_read_or_write, Client& clnt);
	int			fd;
	std::string	read_or_write;
	Client&		client;
};

class Webserv
{
	private:
		//attributes
		int								_epoll_fd;
		std::map<int, Client>			_clients;//organized by FD 
		std::map<int, Listener>			_lstn_sockets;//organized by FD
		std::vector<ServerConfig>		_servers;  
		Router 							_router; 
		Logger							_logger;
		static volatile sig_atomic_t	_flag_sigint;
		int								_waitTime;

		//methods
		void					_setUpSocket(Listener *lstnr);
		addrinfo*				_getAddressFromOS(Listener *lstnr);
		int 					_runServer(void);
		void 					_acceptClient(const Listener& listener);
		int 					_handleClient(Client& client, bool call_recv);
		void					_sendResponse(Client& client);
		std::vector<Listener>	_processParsedPorts(void);
		const ServerConfig& 	_getServerForClient(const Client& client);
		void					_setEpollWaitTime(void);
		void					_timeout(void);
		void                    _checkConnectionClose(Client& client);
		//canonical
		Webserv(const Webserv& src);
		Webserv& operator=(const Webserv& src);
		
	public:
		//attributes
		std::map<int, FdInfo>	cgi_fds;//organized by FD
		//methods
		int 		startServer(void);
		static void handleSigint(int);
		int 		addToEpoll(int fd, uint32_t flags);
		void 		handleError(Client& client, int err_status, std::string err_msg);
		//getters & setters
		void								setServers(std::vector<ServerConfig> servers);
		const std::vector<ServerConfig>&	getServers(void) const;
		Logger& 							getLogger(void);
		int		 							getEpollFd(void);
		void								closeSockets(void);
		void 								closeClient(Client& client);


		//canonical
		Webserv(void);
		~Webserv(void);
		static Webserv& getInstance();
};

#endif