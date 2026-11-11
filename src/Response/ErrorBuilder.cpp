#include "Response.hpp"
#include "Client.hpp"
#include "utils.hpp" 

#include <sstream> 

std::string loadDefaultErrorPage(int code);

void Response::buildError(int code, const ServerConfig& serv, Client& client) 
{
	client.response.setStatus(code);
	client.response.setHead("Content-Type", "text/html");
	std::string body;

	std::map<int, std::string>::const_iterator it = serv.error_pages.find(code);
    if (it != serv.error_pages.end())
		body = readFile(it->second);
	else if (!serv.errorPageDir.empty()) 
		body = readFile(serv.errorPageDir + "/" + intToString(code) + ".html");

	if (body.empty()) 
		body = loadDefaultErrorPage(code);

    client.response.setBody(body);
}

std::string loadDefaultErrorPage(int code) 
{
	if (code == BAD_REQUEST) return "<h1>400 Bad Request</h1>";
	if (code == FORBIDDEN) return "<h1>403 Forbidden</h1>";
    if (code == NOT_FOUND)	return "<h1>404 Not Found</h1>";
	if (code == NOT_ALLOWED) return "<h1>405 Method Not Allowed</h1>";
	if (code == NOT_ACCEPTABLE) return "<h1> 406 Not acceptable</h1>";
	if (code == REQUEST_TIMEOUT) return "<h1> 408 Request Timeout</h1>";
	if (code == CONFLICT) return "<h1> 409 Conflict</h1>";
	if (code == LENGTH_REQUIRED) return "<h1> 411 No Content Length</h1>";
	if (code == TOO_LARGE) return "<h1>413 Payload Too Large</h1>";
	if (code == UNSUPPORTED) return "<h1>415 Unsupported Media Type</h1>";
	if (code == SERVER_ERROR)	return "<h1>500 Internal Server Error</h1>";
	if (code == NOT_IMPLEMENTED) return "<h1>501 Not Implemented</h1>";
	if (code == BAD_GATEWAY) return "<h1>502 Bad Gateway<h1>";
	if (code == GATEWAY_TIMEOUT) return "<h1>504 Gateway Timeout</h1>";
	if (code == VERSION_NOT_SUPPORTED) return "<h1> 505 HTTP version not supported</h1>";
	return "<h1>Error</h1>";
}
