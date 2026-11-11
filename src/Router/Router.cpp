#include "Router.hpp"
#include "utils.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Logger.hpp"
#include "Webserv.hpp"
#include <sstream>
#include <iostream>

int Router::_handleRedirection(Client& client, const LocationConfig& loc)
{
	client.response.setStatus(loc.returnCode);
	client.response.setHead("Location", loc.returnPath);
    client.state = CLOSING;
    client.response.setResponseToSend(true);
    if (client.setEpollOut(true) == 1)
		Webserv::getInstance().closeClient(client);
	return 0;
}

int Router::handleRequest(const std::vector<ServerConfig>& servers, Client& client) 
{
	Request req = client.request;
	std::string path = req.getTarget();
	std::cout << path << std::endl; 

	// find a matching servber
	const ServerConfig* server = findServer(servers, client);
	if (!server)
		throw HttpException(NOT_FOUND, "No matching server");

	// find a matching location
	const LocationConfig* loc = matchLocation(*server, path);
	if (!loc)
		throw HttpException(NOT_FOUND, "Not found");

	// redir
	if (!loc->returnPath.empty())
		return (_handleRedirection(client, *loc));
	// Method validation
	std::string method = req.getMethod();
 	if (validateMethod(loc, method, path) == false)
		throw HttpException(NOT_ALLOWED, "Method not found");

	// Build path
	std::string fullPath = buildFullPath(path, *server, *loc);
	std::cout << "Trying to open: " << fullPath << std::endl;
	
	// handle CGI	
	if (isCgi(*loc, fullPath) == true)
	{
		if (method == "DELETE")
			throw HttpException(NOT_ALLOWED, "Method not found");
		if (handleCgi(client, fullPath, *loc) == 1)
			return (1);
		return (0);
	}
	// Handlers: routing according http method
	if (method == "POST") {
		handlePost(*loc, fullPath, client);
	}
	else if (method == "DELETE") {
		handleDelete(fullPath, client);
	}
	else if (method == "GET") {
		handleGet(*loc, fullPath, client);
	}
	client.state = WRITING;
	client.response.setResponseToSend(true);
	if (client.setEpollOut(true) == 1)
		Webserv::getInstance().closeClient(client);
	return (0);
}

