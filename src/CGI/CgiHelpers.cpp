#include "CgiHandler.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

char** CgiHandler::buildEnv(Client& client, const std::string& fullPath)
{
    const Request& req = client.request;

    std::vector<std::string> temp;

    temp.push_back("REQUEST_METHOD=" + req.getMethod());
    temp.push_back("SCRIPT_NAME=" + req.getTarget());
    temp.push_back("QUERY_STRING=" + req.getQueryString());
    temp.push_back("CONTENT_LENGTH=" + intToString(req.getBody().size()));
    if (req.getHeaders().find("Content-Length") != req.getHeaders().end()) 
        temp.push_back("CONTENT_TYPE=" + req.getHeaders().at("Content-Type"));
    temp.push_back("SERVER_PROTOCOL=" + req.getProtocol());
    temp.push_back("PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
    temp.push_back("REDIRECT_STATUS=200");
    std::string scriptName = fullPath.substr(fullPath.find_last_of('/') + 1);
    temp.push_back("SCRIPT_FILENAME=" + scriptName);
    temp.push_back("SERVER_PORT=" + intToString(client.getLocalPort()));
    temp.push_back("SERVER_SOFTWARE=webserv");

    char **env = new char*[temp.size() + 1];
    for (size_t i = 0; i < temp.size(); i++)
    {
        env[i] = strdup(temp[i].c_str());
        if (!env[i])
        {
            for (size_t j = 0; j < i; j++)
                free(env[j]);
            delete[] env;
            return NULL;
        }
    }

    env[temp.size()] = NULL;
    return env;
}

char** CgiHandler::buildArgs(const std::string& fullPath, const LocationConfig& loc)
{
    std::vector<std::string> temp;
    
    for (std::map<std::string, std::string>::const_iterator handlersIt = loc.cgiHandlers.begin();
        handlersIt != loc.cgiHandlers.end();
        ++handlersIt)
    {
        if (fullPath.find(handlersIt->first) != std::string::npos)
            temp.push_back(handlersIt->second);
    }
    if (temp.size() == 0)
		throw HttpException(NOT_ACCEPTABLE, "Script not allowed\n");
    std::string scriptName = fullPath.substr(fullPath.find_last_of('/') + 1);
    temp.push_back(scriptName);
    char **args = new char*[temp.size() + 1];
    for (size_t i = 0; i < temp.size(); i++)
    {
        args[i] = strdup(temp[i].c_str());
        if (!args[i])
        {
            for (size_t j = 0; j < i; j++)
                free(args[j]);
            delete[] args;
            return NULL;
        }
    }
    args[temp.size()] = NULL;
    return args;
}

void CgiHandler::cleanExecute(char **env, char **args)
{
    if (env)
    {
        for (size_t i = 0; env[i]; i++)
        free(env[i]);
        delete[] env;
    }
    if (args)
    {
        for (size_t i = 0; args[i]; i++)
        free(args[i]);
        delete[] args;
    }
}

void	CgiHandler::removePipeFromEpoll(int fd, Client& client)
{
	Webserv& webserv = Webserv::getInstance();
	
	epoll_ctl(webserv.getEpollFd(), EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	if (fd == client.cgi.read_end)
		client.cgi.read_end_closed = true;
	else if (fd == client.cgi.write_end)
		client.cgi.write_end_closed = true;
	webserv.cgi_fds.erase(fd);
}

void CgiHandler::_buildCgiResponse(Client& client, std::string cgiOutput)
{
    size_t pos = 0;
    while ((pos = cgiOutput.find("\r\n", pos)) != std::string::npos)
        cgiOutput.replace(pos, 2, "\n"); //apparently, the separators might vary depending the type of script

    size_t header_end = cgiOutput.find("\n\n"); 
    if (header_end == std::string::npos)
    {
        client.state = CLOSING;
        Webserv& webserv = Webserv::getInstance();
        webserv.handleError(client, BAD_GATEWAY, "CGI: no header end found");
        client.response.setResponseToSend(true);
        if (client.setEpollOut(true) == 1)
            webserv.closeClient(client);
        return;
    }
    std::string headers = cgiOutput.substr(0, header_end);
    std::string body = cgiOutput.substr(header_end + 2);

    const std::string CONTENT_TYPE = "Content-Type: ";
    size_t contentPos = headers.find(CONTENT_TYPE);
    if (contentPos == std::string::npos)
    {
        client.state = CLOSING;
        Webserv& webserv = Webserv::getInstance();
        webserv.handleError(client, BAD_GATEWAY, "CGI: no Content-Type");
        client.response.setResponseToSend(true);
        if (client.setEpollOut(true) == 1)
            webserv.closeClient(client);
        return;
    }
    size_t typeEnd = headers.find("\n", contentPos);
    std::string type = headers.substr(contentPos + CONTENT_TYPE.size(),
                                      typeEnd - contentPos - CONTENT_TYPE.size());
    client.response.setHead("Content-Type", type);

    const std::string STATUS = "Status: ";
    size_t statusPos = headers.find(STATUS);
    if (statusPos != std::string::npos)
    {
        size_t statusEnd = headers.find("\n", statusPos);
        std::string statusLine = headers.substr(statusPos + STATUS.size(),
                                                statusEnd - statusPos - STATUS.size());
        std::istringstream iss(statusLine);
        int code;
        iss >> code;
        client.response.setStatus(code);
    }
    else
        client.response.setStatus(OK);
    client.response.setBody(body);
    client.state = WRITING;
    client.response.setResponseToSend(true);
    if (client.setEpollOut(true) == 1)
        Webserv::getInstance().closeClient(client);
}
