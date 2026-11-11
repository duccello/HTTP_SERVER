#include "Webserv.hpp"
#include "Client.hpp"
#include "CgiHandler.hpp"
#include <ctime>

void Webserv::_timeout()
{
    size_t currentTime = static_cast<size_t>(time(NULL));

    std::map<int, Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        Client& client = it->second;
        std::map<int, Client>::iterator next = it;
        ++next;

        const ServerConfig* server = _router.findServer(getServers(), client);
        size_t timeout = server->timeout;

        if (client.state == CGI && currentTime - client.cgi.start_time > timeout)
        {
            std::cerr << "Killing CGI Process because of GATEWAY_TIMEOUT\n";
            if (!client.cgi.read_end_closed)
                CgiHandler::removePipeFromEpoll(client.cgi.read_end, client);
            if (!client.cgi.write_end_closed)
                CgiHandler::removePipeFromEpoll(client.cgi.write_end, client);
            kill(client.cgi.pid, SIGKILL);
            client.response.buildError(GATEWAY_TIMEOUT, *server, client);
            client.response.setResponseToSend(true);
            client.state = CLOSING;
            if (client.setEpollOut(true) == 1)
                closeClient(client);
        }
        else if (client.state == READING && currentTime - client.lastEvent > timeout)
        {
            std::cerr << "Request Timeout" << std::endl;
            client.response.buildError(REQUEST_TIMEOUT, *server, client);
            client.response.setResponseToSend(true);
            client.state = CLOSING;
            if (client.setEpollOut(true) == 1)
                closeClient(client);        
        }
        else if (client.state == WRITING && currentTime - client.lastEvent > timeout)
        {
            closeClient(client);
        }
        it = next;
    }
}

void Webserv::_setEpollWaitTime()
{
    _waitTime = _servers[0].timeout * 1000;
    for (size_t i = 1; i < _servers.size(); i++)
    {
        if (_servers[i].timeout < _waitTime / 1000)
            _waitTime = _servers[i].timeout * 1000;
    }
    _waitTime = _waitTime * 0.3;
}