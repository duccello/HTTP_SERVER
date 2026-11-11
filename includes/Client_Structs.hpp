#ifndef CLIENT_STRUCTS_HPP
#define CLIENT_STRUCTS_HPP

# include <iostream>

class Response;

enum State
{
    WAITING,
    READING,
    HANDLING,
    WRITING,
    CGI,
    CLOSING
};

struct CgiData 
{
	CgiData(void);

    pid_t		pid;
    int			read_end;   // parent reads to CGI
    int 		write_end;  // parent writes from CGI

    std::string	body_to_send;
    size_t		body_sent;

    std::string	output;

    bool		write_end_closed;
    bool		read_end_closed;

    size_t 		start_time;
};

#endif