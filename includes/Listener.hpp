#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <string>

//each one is a listening socket

class Listener
{
	private:
		//attributes
		int					_fd;
		const int			_port;
		std::string			_interface;
		//canonical
		Listener(void);
		Listener& operator=(const Listener& src);

	public:
		//canonical
		Listener(int port, const std::string& interface);
		Listener(const Listener& src);
		~Listener(void);
		//getters & setters
		int			getPort() const;
		int			getFD() const;
		void		setFD(int fd);
		const std::string& 		getInterface() const;	
};

#endif