#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <iostream>
# include <unistd.h>
# include <sys/epoll.h>
# include "Response.hpp"
# include "Request.hpp"
# include "Client_Structs.hpp"

class Response;

class Client
{
	private:
		//methods
		Client(void);
		Client& operator=(const Client& src);
		//attributes
		const int _fd;			// fd used on the server side for direct communication with this client
		const int _listener_fd; // which server fd (port) accepted this client
		const int _local_port;
		
	public:
        //constr + destr
		Client(int fd, int listener_fd, int port);
        Client(const Client& src);
        ~Client(void);
		
		//attributes
		std::string	buffer;
		Request		request;
		Response	response;
		CgiData		cgi;
		State		state;
		size_t		lastEvent;

        //getters
        int			getFD(void) const;
		int			getListener(void) const;
		int 		getLocalPort() const;
		int 		setEpollOut(bool enable);
};

#endif