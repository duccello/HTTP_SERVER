#include "CgiHandler.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

int	CgiHandler::sendRequestToCgi(Client& client)
{
	CgiData& cgi = client.cgi;
	size_t		body_size = cgi.body_to_send.size();

    ssize_t just_sent = write(cgi.write_end, 
				cgi.body_to_send.c_str() + cgi.body_sent,
            	body_size - cgi.body_sent);
    if (just_sent > 0)
	{
        cgi.body_sent += just_sent;
		if (cgi.body_sent < body_size)
		{
			return (0);
		}
		else
		{
			removePipeFromEpoll(cgi.write_end, client);
			return (0);
		}
	}
    else
	{
		kill(cgi.pid, SIGKILL);
		Webserv::getInstance().handleError(client, SERVER_ERROR, "write(): error sending request to cgi");
		waitAndFinishCgi(client);
        return (0);
    }
}

int	CgiHandler::receiveCgiOutput(Client& client)
{
	CgiData& cgi = client.cgi;
    char buffer[4096];
    ssize_t bytes;

	bytes = read(cgi.read_end, buffer, sizeof(buffer));

	if (bytes > 0)
	{
		cgi.output.append(buffer, bytes);
		return (0);
	}
	else
	{
		kill(cgi.pid, SIGKILL);
		Webserv::getInstance().handleError(client, SERVER_ERROR, "read(): error receiving cgi output");
		waitAndFinishCgi(client);
	}
	return (0);
}

int CgiHandler::waitAndFinishCgi(Client& client)
{
    int status;
    pid_t result = waitpid(client.cgi.pid, &status, WNOHANG);
    
    if (result == 0)
        return (0);
    else if (result > 0)
    {
        uint8_t ret_val = get_return_val(status);
        if (ret_val != 0)
		{
			client.state = CLOSING;
			if (client.response.getStatus() == 0)
            	Webserv::getInstance().handleError(client, BAD_GATEWAY, "CGI exited with error");
		}
		else
		{
			_buildCgiResponse(client, client.cgi.output);
		}
	}
    else
	{
		client.state = CLOSING;
        Webserv::getInstance().handleError(client, SERVER_ERROR, "waitpid() failed");
	}

    if (!client.cgi.read_end_closed)
		removePipeFromEpoll(client.cgi.read_end, client);
	if (!client.cgi.write_end_closed)
		removePipeFromEpoll(client.cgi.write_end, client);
	if (client.state != CLOSING)
        client.state = WRITING;
    client.response.setResponseToSend(true);
    client.cgi = CgiData();
    if (client.setEpollOut(true) == 1)
		Webserv::getInstance().closeClient(client);
    return (0);
}
