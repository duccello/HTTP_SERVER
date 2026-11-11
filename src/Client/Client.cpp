#include "Client.hpp"
#include "Webserv.hpp"


int Client::setEpollOut(bool enable)
{
	epoll_event event;
	int			epoll_fd = Webserv::getInstance().getEpollFd();

	event.events = EPOLLERR | EPOLLHUP;
	if (enable)
		event.events |= EPOLLOUT;
	else
		event.events |= EPOLLIN;

	event.data.fd = _fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, _fd, &event) == -1)
		return (1);
	return (0);	
}