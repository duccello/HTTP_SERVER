#include "Webserv.hpp"
#include "utils.hpp"

addrinfo* Webserv::_getAddressFromOS(Listener *lstnr)
{
	addrinfo	hints;
	addrinfo	*res;
	int			ret;

	std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    ret = getaddrinfo(lstnr->getInterface().c_str(), (intToString(lstnr->getPort())).c_str(), &hints, &res);
    if (ret != 0) 
		throw std::runtime_error(std::string("getaddrinfo: ")
				+ gai_strerror(ret) + "\n");
	return (res);
}

void Webserv::_setUpSocket(Listener *lstnr)
{
	addrinfo	*res;
	addrinfo	*p;
	int			opt = 1;

	res = Webserv::_getAddressFromOS(lstnr);
    for (p = res; p != NULL; p = p->ai_next) 
	{
        lstnr->setFD(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (lstnr->getFD() == -1)
			continue;
        setsockopt(lstnr->getFD(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (bind(lstnr->getFD(), p->ai_addr, p->ai_addrlen) == 0) 
            break;
        close(lstnr->getFD());
    }
    freeaddrinfo(res);
	if (lstnr->getFD() < 0)
		throw std::runtime_error("could not create listening socket\n");
    if (p == NULL)
	{
		throw std::runtime_error(std::string("bind: ") + strerror(errno) + "\n");
	}
    if (listen(lstnr->getFD(), SOMAXCONN) == -1)
	{
		close (lstnr->getFD());
		throw std::runtime_error(std::string("listen: ") + strerror(errno) + "\n");
	}

	int flags = fcntl(lstnr->getFD(), F_GETFL, 0);
	
	fcntl(lstnr->getFD(), F_SETFL, flags | O_NONBLOCK);
    std::cout << "Listening on http://" << lstnr->getInterface() << ":" << lstnr->getPort() << "\n";
	return;
}


std::vector<Listener> Webserv::_processParsedPorts(void)
{
	std::map<int, std::string>				uniquePorts;
	std::vector<Listener>				parsed_listeners;
    const std::vector<ServerConfig>&	servers = this->getServers();

    for (std::vector<ServerConfig>::const_iterator srv = servers.begin();
         srv != servers.end();
         ++srv)
    {
        for (std::vector<ListenDirective>::const_iterator lst = srv->ports.begin();
			 lst != srv->ports.end();
			 ++lst)
        {
            uniquePorts.insert(std::make_pair(lst->port, lst->interface));
        }
    }
	for (std::map<int, std::string>::const_iterator it = uniquePorts.begin();
     it != uniquePorts.end();
     ++it)
	{
		Listener temp(it->first, it->second);
    	parsed_listeners.push_back(temp);
	}
    return parsed_listeners;
}
