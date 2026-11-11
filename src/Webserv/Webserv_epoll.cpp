#include "Webserv.hpp"
#include "utils.hpp"

int Webserv::addToEpoll(int fd, uint32_t flags)
{
    epoll_event event;
    std::memset(&event, 0, sizeof(event));

    event.events = EPOLLERR | EPOLLHUP | flags;
    event.data.fd = fd;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
        return (1);
    return (0);
}
