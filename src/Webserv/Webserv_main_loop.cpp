#include "Webserv.hpp"
#include "utils.hpp"
#include "CgiHandler.hpp"

int Webserv::_runServer(void)
{
	epoll_event events[64];
	int ready;
	int fd;

	while (!Webserv::_flag_sigint)
	{
		ready = epoll_wait(_epoll_fd, events, 64, _waitTime);
		
        if (ready == -1)
		{
            if (errno == EINTR)
                continue;
            std::cerr << "epoll_wait: " << std::strerror(errno) << "\n";
            continue;
        }
        for (int i = 0; i < ready; ++i)
		{
            fd = events[i].data.fd;

            if (events[i].events & EPOLLIN)
			{
				std::cout << "EPOLLIN\n";
				std::map<int, Listener>::iterator itL = _lstn_sockets.find(fd);
				if (itL != _lstn_sockets.end())
				{
					_acceptClient(itL->second);
				}
				else
				{
					std::map<int, Client>::iterator itC = _clients.find(fd);

					if (itC != _clients.end())
						std::cout << "epollin is from a client, state " << itC->second.state << "\n";

					if (itC != _clients.end() && (itC->second.state == WAITING || itC->second.state == READING))
					{
						if (_handleClient(itC->second, true) == 1)
							return (1);
					}
					else if (itC == _clients.end())
					{
						std::map<int, FdInfo>::iterator itCGI = cgi_fds.find(fd);
						
						if (itCGI != cgi_fds.end())
							std::cout << "epollin is from cgi, state " << itCGI->second.client.state << "\n";

						if (itCGI != cgi_fds.end() && itCGI->second.client.state == CGI)
						{
							CgiHandler::receiveCgiOutput(itCGI->second.client);
						}
					}
				}
			}
			if (events[i].events & EPOLLOUT)
			{
				std::cout << "EPOLLOUT\n";
				std::map<int, Client>::iterator itR = _clients.find(fd);

				if (itR != _clients.end())
					std::cout << "epollout is from a client, state " << itR->second.state << "\n";

				if (itR != _clients.end() && 
				(itR->second.state == WRITING ||
				(itR->second.state == CLOSING && itR->second.response.getResponseToSend() == true)))
				{
					_sendResponse(itR->second);
				}
				else if (itR == _clients.end())
				{
					std::map<int, FdInfo>::iterator it = cgi_fds.find(fd);
					if (it != cgi_fds.end() && it->second.client.state == CGI)
					{
						std::cout << "epollout is from cgi, state " << it->second.client.state << "\n";
						CgiHandler::sendRequestToCgi(it->second.client);
					}
				}
			}
			if (events[i].events & (EPOLLERR | EPOLLHUP))
			{
				if (events[i].events & EPOLLERR)
					std::cout << "EPOLLERR\n";
				else if (events[i].events & EPOLLHUP)
					std::cout << "EPOLLHUP\n";
					
                if (_clients.count(fd))
                    closeClient(_clients.find(fd)->second);
				else 
				{
					std::map<int, FdInfo>::iterator itERR = cgi_fds.find(fd);
					if (itERR != cgi_fds.end()
					&& itERR->second.client.state == CGI)
					{
						CgiHandler::waitAndFinishCgi(itERR->second.client);
					}
				}
            }
			std::map<int, Client>::iterator itClose = _clients.find(fd);
			if (itClose != _clients.end()
				&& itClose->second.state == CLOSING
				&& itClose->second.response.getResponseToSend() == false)
			{
				closeClient(itClose->second);
			}
        }
		_timeout();
	}
	return (0);
}

int Webserv::startServer(void)
{
	std::vector<Listener>	parsed_listeners(_processParsedPorts());

	for (std::vector<Listener>::iterator it = parsed_listeners.begin();
		it != parsed_listeners.end(); ++it)
	{
		_setUpSocket(&(*it));
		Listener to_add((*it));
		this->_lstn_sockets.insert(std::make_pair((*it).getFD(), to_add));
	}
    _epoll_fd = epoll_create1(0);
	
    if (_epoll_fd == -1)
		throw std::runtime_error(std::string("epoll_create1: ") + strerror(errno) + "\n");
    for (std::map<int, Listener>::iterator it = _lstn_sockets.begin();
		it != _lstn_sockets.end(); ++it)
	{
        if (addToEpoll(it->first, EPOLLIN) == 1)
		{
			throw std::runtime_error(std::string("epoll_ctl ADD listener fd ") + intToString(it->first)
            		+ ": " + strerror(errno) + "\n");
		}
    }
	_setEpollWaitTime();
	return (_runServer());
}