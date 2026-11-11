#include "Webserv.hpp"

Webserv::Webserv(void) 
	: _epoll_fd(-2),
	  _router(),
	  _logger("error.log") {
}

Webserv::~Webserv(void)
{
	closeSockets();
}

Webserv& Webserv::getInstance()
{
    static Webserv instance;
    return instance;
}

const std::vector<ServerConfig>& Webserv::getServers() const
{
	return (_servers);
}

void Webserv::setServers(std::vector<ServerConfig> servers)
{
	this->_servers = servers;
}

Logger& Webserv::getLogger(void)
{
    return _logger;
}

int Webserv::getEpollFd(void)
{
    return _epoll_fd;
}

FdInfo::FdInfo(int pipefd, std::string to_read_or_write, Client& clnt): 
								fd(pipefd), read_or_write(to_read_or_write), client(clnt)
{
}