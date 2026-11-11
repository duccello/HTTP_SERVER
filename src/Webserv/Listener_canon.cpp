#include "Listener.hpp"
#include <unistd.h>

Listener::Listener(int port, const std::string& interface) : _fd(-2), _port(port), _interface(interface)
{
}

Listener::~Listener(void)
{
}

Listener::Listener(const Listener& src) : _fd(src._fd), _port(src._port), _interface(src._interface)
{
}

int Listener::getPort() const
{
	return (this->_port);
}

int Listener::getFD() const
{
	return (this->_fd);
}

const std::string& Listener::getInterface() const
{
	return _interface;
}

void Listener::setFD(int fd)
{
	this->_fd = fd;
}