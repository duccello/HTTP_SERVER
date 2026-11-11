#include "Webserv.hpp"
#include "CgiHandler.hpp"

void Webserv::closeClient(Client& client)
{
    std::cout << "closing client " << client.getFD() << "\n"; //DEBUGGER
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client.getFD(), NULL);
    close(client.getFD());
    if (client.cgi.read_end_closed == false)
        CgiHandler::removePipeFromEpoll(client.cgi.read_end, client);
    if (client.cgi.write_end_closed == false)
        CgiHandler::removePipeFromEpoll(client.cgi.write_end, client);    
    _clients.erase(client.getFD());
}

void Webserv::closeSockets(void)
{
	for (std::map<int, Listener>::iterator it = this->_lstn_sockets.begin();
    	it != this->_lstn_sockets.end(); ++it)
		close(it->first);
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
                close(it->first);
	if (_epoll_fd >= 0)
                close(_epoll_fd);
}
