#include "Webserv.hpp"
#include "Request.hpp"

void Webserv::_acceptClient(const Listener& listener)
{
    sockaddr_storage	client_addr;
    socklen_t			client_len = sizeof(client_addr);

	std::memset(&client_addr, '0', client_len);
    int client_fd = accept(listener.getFD(), (sockaddr*)&client_addr, &client_len);
    if (client_fd == -1)
		throw std::runtime_error(std::string("accept: ") + strerror(errno) + "\n");
	
	// DELETE
	// int size = 4096; // these three lines test sending in chunks
	// setsockopt(client_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
	// std::cout << "client SO_SNDBUF = " << size << std::endl;
	
	int flags = fcntl(client_fd, F_GETFL, 0);
	fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    Client new_clnt(client_fd, listener.getFD(), listener.getPort());
    _clients.insert(std::make_pair(new_clnt.getFD(), new_clnt));

    if (addToEpoll(client_fd, EPOLLIN) == 1)
        closeClient(new_clnt);
    else
    {
        std::cout << "Accepted client fd " << client_fd
                  << " from fd " << listener.getFD()
                  << " (port " << listener.getPort() << ")\n";
    }
}

int Webserv::_handleClient(Client& client, bool call_recv)
{
    if (call_recv)
    {
        char buffer[4096];
        std::memset(&buffer, 0, 4096);
        ssize_t bytes = recv(client.getFD(), buffer, sizeof(buffer), 0);
    
        if (bytes <= 0)
        {
            closeClient(client);
            return (0);
        }
        client.buffer.append(buffer, bytes);
    }
    client.state = READING;
    client.lastEvent = static_cast<size_t>(time(NULL));

    if (client.request.areHeadersComplete(client.buffer))
    {
        try
        {
            if (!client.request._headers_parsed)
            {
                client.request.parseHeaders(client.buffer);
                client.request.isValid();
                client.request._headers_parsed = true;
                _router.checkBodySize(this->getServers(), client);
                _router.handleExpect(client);
            }
            if (!client.request.isBodyComplete(client.buffer))
                return (0);
            client.state = HANDLING;
            client.request.parseBody(client.buffer);
            if (_router.handleRequest(this->getServers(), client) == 1)
                return (1);
        }
        catch (HttpException& e)
        {
            handleError(client, e.getStatus(), e.what());
            if (client.state != CLOSING)
                client.state = WRITING;
            client.response.setResponseToSend(true);
            if (client.setEpollOut(true) == 1)
                closeClient(client);
        }
    }
    return (0);
}

void Webserv::handleError(Client& client, int err_status, std::string err_msg)
{
	if (err_status == 413 || err_status == 400)
		client.state = CLOSING;
	const ServerConfig* targetServer = _router.findServer(this->getServers(), client);
    client.response.buildError(err_status, *targetServer, client); 
    _logger.logError(err_status, err_msg);
    std::cerr << err_msg << "\n";
}

void Webserv::_checkConnectionClose(Client& client)
{
    std::map<std::string, std::string> headers = client.request.getHeaders();
    std::map<std::string, std::string>::const_iterator it = 
                                        headers.find("Connection");
    if (it != headers.end() && it->second == "close")
    {
        client.state = CLOSING;
    }
}

void Webserv::_sendResponse(Client& client)
{
	Response&	res = client.response;
	int			fd = client.getFD();
	std::string raw;
	int			sent;
	
	_checkConnectionClose(client);
    if (client.state == CLOSING)
		client.response.setHead("Connection", "close");
	raw = res.toString();
	sent = send(fd, raw.c_str() + res.getBytesSent(),
				raw.size() - res.getBytesSent(), 0);
	if (sent <= 0)
		closeClient(client);
	else
	{
        client.lastEvent = static_cast<size_t>(time(NULL));
		res.setBytesSent(res.getBytesSent() + sent);
		if (res.getBytesSent() < raw.size())
			return;
		else if (client.setEpollOut(false) == 1)
			closeClient(client);
		else
		{
			std::cout << "response successfully sent\n"; //DEBUG
			if (client.state == CLOSING)
			{
				closeClient(client);
				return ;
			}
            if (client.response.getStatus() != CONTINUE)
            {
                client.buffer.erase(0, client.request.getSizeInclHeader());
                client.request = Request();
            }
   			client.response = Response();
			if (client.buffer.empty())
            {
                client.state = WAITING;
                return;
            }
            else
            {
                client.lastEvent = static_cast<size_t>(time(NULL));
                client.state = HANDLING;
                _handleClient(client, false);
            }
		}
	}
}
