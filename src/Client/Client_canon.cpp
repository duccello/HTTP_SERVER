#include "Client.hpp"

Client::Client(int fd, int listener_fd, int port) :
								_fd(fd), _listener_fd(listener_fd), _local_port(port),
								state(WAITING)
{
    lastEvent = static_cast<size_t>(time(NULL));
}

Client::~Client(void)
{
}

Client::Client(const Client& src) : _fd(src._fd), _listener_fd(src._listener_fd),
    							 _local_port(src._local_port), buffer(src.buffer),
								 cgi(src.cgi), state(src.state), lastEvent(src.lastEvent)
{
}

int Client::getLocalPort(void) const
{
	return (this->_local_port);
}

int Client::getFD(void) const
{
	return (this->_fd);
}

int Client::getListener(void) const
{
	return (this->_listener_fd);
}

CgiData::CgiData(void) : read_end(-1), write_end(-1), body_sent(0),
						write_end_closed(true), read_end_closed(true), start_time(0)
{	
}
