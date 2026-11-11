#include "CgiHandler.hpp"
#include "Webserv.hpp"

int CgiHandler::pipeAndFork(Client& client, const std::string &fullPath, const LocationConfig& loc)
{
    char    **env = buildEnv(client, fullPath);
    char    **args = buildArgs(fullPath, loc);
    if (env == NULL || args == NULL)
    {
        cleanExecute(env, args);
		throw HttpException(SERVER_ERROR, "Allocation error in CGI handler");
    }

    int in[2];
    int out[2];
    if (pipe(in) == -1)
    {
        cleanExecute(env, args);
        throw HttpException(SERVER_ERROR, "Piping error in CGI handler");
    }   
    if (pipe(out) == -1)
    {
        close(in[READ_END]);
        close(in[WRITE_END]);
        cleanExecute(env, args);
        throw HttpException(SERVER_ERROR, "Piping error in CGI handler");
    }

    int pid = fork();
    if (pid < 0)
    {
        cleanExecute(env, args);
        throw HttpException(SERVER_ERROR, "Forking error in CGI handler");
    }
    else if (pid == 0)
    {
   		std::string scriptDir = fullPath.substr(0, fullPath.find_last_of('/'));
        std::cout << "scriptDir: " << scriptDir << std::endl; 
        return (_childProcess(in, out, fullPath, loc, env, args));
    }
    else
		_setupCgiParent(in, out, client, pid);
    cleanExecute(env, args);
    return (0);
}

int CgiHandler::_setupCgiParent(int in[2], int out[2], Client& client, int pid)
{
	    Webserv& webserv = Webserv::getInstance();

        close(in[READ_END]);
        close(out[WRITE_END]);
        fcntl(in[WRITE_END], F_SETFL, O_NONBLOCK);
        fcntl(out[READ_END], F_SETFL, O_NONBLOCK);

        client.cgi.pid = pid;
        client.cgi.write_end = in[WRITE_END];
        client.cgi.write_end_closed = false;
        client.cgi.read_end = out[READ_END];
        client.cgi.read_end_closed = false;

        if (webserv.addToEpoll(out[READ_END], EPOLLIN) == 1)
        {
            kill(client.cgi.pid, SIGKILL);
            Webserv::getInstance().handleError(client, SERVER_ERROR, "read(): error receiving cgi output");
            waitAndFinishCgi(client);
        }
        FdInfo to_add(out[READ_END], "read", client);
        webserv.cgi_fds.insert(std::make_pair(out[READ_END], to_add));

        if (client.request.getMethod() == "POST")
        {
            client.cgi.body_to_send = client.request.getBody();
            if (webserv.addToEpoll(in[WRITE_END], EPOLLOUT) == 1)
            {
                kill(client.cgi.pid, SIGKILL);
                Webserv::getInstance().handleError(client, SERVER_ERROR, "read(): error receiving cgi output");
                waitAndFinishCgi(client);
            }
            FdInfo to_add2(in[WRITE_END], "write", client);
            webserv.cgi_fds.insert(std::make_pair(in[WRITE_END], to_add2));
        }
        else
        {
            close(in[WRITE_END]);
            client.cgi.write_end_closed = true;
        }
        client.cgi.start_time = static_cast<size_t>(time(NULL));
		return (0);
}