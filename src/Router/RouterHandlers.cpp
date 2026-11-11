#include "Response.hpp"
#include "Request.hpp"
#include "Router.hpp"
#include "utils.hpp"
#include "MimeType.hpp"
#include "Autoindex.hpp"

#include "CgiHandler.hpp"
#include <iostream>
#include <cstdio>
#include <errno.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>

void Router::handleAutoindex(const std::string& fullPath, Client& client) {
	std::string body = Autoindex::generateAutoidx(fullPath, client.request.getTarget());
	client.response.setStatus(OK);
	client.response.setHead("Content-Type", "text/html");
	client.response.setBody(body);
}

void Router::handleDelete(const std::string& fullPath, Client& client) 
{
    if (std::remove(fullPath.c_str()) != 0)
    {
        if (errno == EACCES) 
			throw HttpException(FORBIDDEN, "Forbidden"); 
        else if (errno == ENOENT) 
			throw HttpException(NOT_FOUND, "Not Found"); 
		else
			throw HttpException(SERVER_ERROR, "Server Error");
    }
    client.response.setStatus(NO_CONTENT);
}

void Router::handlePost(const LocationConfig& loc, std::string fullPath, Client& client) 
{
	if (client.request.getBody().size() > loc.clientMaxBodySize)
		throw HttpException(TOO_LARGE, "Payload Too Large");
	int outFile = open(fullPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (outFile == -1)
	{
		switch (errno)
		{
		    case EEXIST:
		        throw HttpException(CONFLICT, "File already exists");
			case ENOENT:
				throw HttpException(NOT_FOUND, "Directory does not exist");
			case EACCES:
				throw HttpException(FORBIDDEN, "Permission denied");
			default:
				throw HttpException(SERVER_ERROR, "Server Error");
		}
	}
	if (writeLoop(outFile, client.request.getBody()) == -1)
	{
	    close(outFile);
		throw HttpException(SERVER_ERROR, "Server Error");
	}
	close(outFile);
	client.response.setStatus(CREATED);
}

void Router::handleGet(const LocationConfig& loc, const std::string& fullPath,  Client& client) 
{
	struct stat st;

	// 1. Check if path exists and get its metadata
	if (stat(fullPath.c_str(), &st) != 0)
		throw HttpException(NOT_FOUND, "Not Found");
	
	if (S_ISDIR(st.st_mode)) {
		if (loc.autoidx) {
			handleAutoindex(fullPath, client);
			return;
		} else {
			throw HttpException(NOT_FOUND, "Not Found");
		}
	}

	// 3. It's a file → serve it
	//3.1. read the file
	std::string body = readFile(fullPath);

	stat(fullPath.c_str(), &st);

	// 3.2. Autoindex (if there is no file or if is empty)
	if (body.empty()) 
	{
		if (loc.autoidx) 
		{
			client.response.setStatus(OK);
			client.response.setHead("Content-Type", "text/html");
			client.response.setBody("<h1>Directory Listing (TODO)</h1>");
		}
		else
			throw HttpException(NOT_FOUND, "Not Found");
	}

	// 3.3. Det Content-Type (MIME type)
	std::string contentType = MimeType::getType(fullPath);

	// 3.4. Success response
	client.response.setStatus(OK);
	client.response.setHead("Content-Type", contentType);
	client.response.setBody(body);
}

int	Router::handleCgi(Client& client, const std::string& fullPath, const LocationConfig& loc)
{
	if (access(fullPath.c_str(), F_OK) != 0)
	{
		switch (errno)
		{
            case ENOENT:
                throw HttpException(NOT_FOUND, "script not found");
            case EACCES:
                throw HttpException(FORBIDDEN, "Inaccesible");
            case ENOTDIR:
                throw HttpException(NOT_FOUND, "No directory");
            default:
                throw HttpException(SERVER_ERROR, "Server Error");
		}
	}
	client.state = CGI;
	return (CgiHandler::pipeAndFork(client, fullPath, loc));
}
