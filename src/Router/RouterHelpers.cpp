#include "Router.hpp"
#include "utils.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Logger.hpp"
#include "MimeType.hpp"
#include "Webserv.hpp"
#include <sstream>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>

static bool perfectMatch(const std::string& path, const std::string& loc);

const ServerConfig* Router::findServer(const std::vector<ServerConfig>& servers, const Client& client)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        for (size_t j = 0; j < servers[i].ports.size(); j++)
        {
            if (client.getLocalPort() == servers[i].ports[j].port)
                return &servers[i];
        }
    }
    return NULL;
}

const LocationConfig* Router::matchLocation(const ServerConfig& server, const std::string& path) 
{
	const LocationConfig* bestMatch = NULL;
		size_t bestLen = 0;

		for (size_t i = 0; i < server.locations.size(); i++) 
		{
			const LocationConfig& loc = server.locations[i];
			if (perfectMatch(path, loc.path) && loc.path.size() > bestLen)
			{
				bestMatch = &loc;
				bestLen = loc.path.size();
			}
		}
		return bestMatch;
}

 static bool perfectMatch(const std::string& path, const std::string& loc)
{
	if (loc == "/")
        return true;
    if (path.substr(0, loc.size()) != loc)
        return false;
    if (path.size() == loc.size())
        return true;
    if (path[loc.size()] == '/')
        return true;
    return false;
} 

std::string Router::buildFullPath(const std::string& path, const ServerConfig& server, const LocationConfig& loc) const 
{
	std::string resolvePath;

	if (path == "/" && !loc.autoidx && !loc.idx.empty())
		resolvePath = std::string("/") + loc.idx;
	else
		resolvePath = path;

	std::string root = server.root;
	if (!root.empty() && root[root.size() - 1] == '/' && !resolvePath.empty() && resolvePath[0] == '/') {
		root.erase(root.size() - 1);
	}

	return root + resolvePath;
}

bool    Router::validateMethod(const LocationConfig* loc, std::string method, const std::string& path)
{
    if (method == "GET") 
    {
        return true;
    }
    if (method == "POST" && path == "/")
    {
        return false;
    }
	for (size_t i = 0; i < loc->methods.size(); i++) 
    {
		if (loc->methods[i] == method) 
        {
            return true;
		}
	}
    return false;
}
const LocationConfig* Router::findLocation(const std::vector<ServerConfig>& servers, const Client& client, const std::string& path)
{
    const ServerConfig* server = findServer(servers, client);
    if (!server)
        throw HttpException(NOT_FOUND, "no matching server");
    return matchLocation(*server, path);
}

void Router::checkBodySize(const std::vector<ServerConfig>& servers, Client& client)
{
    const LocationConfig* loc = findLocation(servers, client, client.request.getTarget());
    if (!loc)
        throw HttpException(NOT_FOUND, "no matching location");

    if (!client.request.getHeaders().count("Content-Length"))
        return;

    size_t content_length = std::strtoul(client.request.getHeaders().at("Content-Length").c_str(), NULL, 10);
    if (content_length > loc->clientMaxBodySize)
        throw HttpException(TOO_LARGE, "body too large");
}

void Router::handleExpect(Client& client)
{
    if (!client.request.getHeaders().count("Expect"))
        return;
    if (client.request.getHeaders().at("Expect") != "100-continue")
        return;
    client.response.setStatus(CONTINUE);
    client.state = WRITING;
    client.response.setResponseToSend(true);
    if (client.setEpollOut(true) == 1)
		Webserv::getInstance().closeClient(client);
}

bool Router::isCgi(const LocationConfig& loc, const std::string& path)
{
    size_t extension = path.rfind(".");
    if (extension == std::string::npos)
        return false;
    std::string CGI = path.substr(extension);
    if (loc.cgiHandlers.count(CGI) > 0)
        return true;
    return false;
}