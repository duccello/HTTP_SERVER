#include "Response.hpp"
#include "Client.hpp"
#include "utils.hpp" 

#include <sstream> 

std::string Response::toString() const 
{
	if (_status == CONTINUE)
		return "HTTP/1.1 100 Continue\r\n\r\n";
	std::stringstream ss; 
	ss << "HTTP/1.1 " << _status << " " << getStatusMsg(_status) << "\r\n";
	ss << "Content-Length: " << _body.size() << "\r\n";
	if (_headers.find("Content-Type") == _headers.end())
		ss << "Content-Type: text/plain\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it) {
		ss << it->first << ": " << it->second << "\r\n";
	}
	ss << "\r\n";

	std::string fullResponse = ss.str();
	fullResponse.append(_body);

	return fullResponse;
}

std::string Response::getStatusMsg(int code) const
{
	if (code == CONTINUE) return "Continue";
	if (code == OK) return "OK";
	if (code == CREATED) return "Created";
	if (code == ACCEPTED) return "Accepted";
	if (code == NO_CONTENT) return "No Content";
	if (code == MOVED_PERMANENTLY) return "Moved Permanently";
	if (code == FOUND) return "Found";
	if (code == SEE_OTHER) return "See Other";
	if (code == TEMPORARY_REDIRECT) return "Temporary Redirect";
	if (code == PERMANENT_REDIRECT) return "Permanent Redirect";
	if (code == BAD_REQUEST) return "Bad Request";
	if (code == FORBIDDEN) return "Forbidden";
	if (code == NOT_FOUND) return "Not Found";
	if (code == NOT_ALLOWED) return "Method Not Allowed";
	if (code == NOT_ACCEPTABLE) return "Not acceptable";
	if (code == REQUEST_TIMEOUT) return "Request_Timeout";
	if (code == CONFLICT) return "Conflict";
	if (code == LENGTH_REQUIRED) return "No Content Length";
	if (code == TOO_LARGE) return "Payload Too Large";
	if (code == UNSUPPORTED) return "Unsupported";
	if (code == SERVER_ERROR) return "Internal Server Error";
	if (code == NOT_IMPLEMENTED) return "Not Implemented";
	if (code == BAD_GATEWAY) return "Bad Gateway";
	if (code == GATEWAY_TIMEOUT) return "Gateway Timeout";
	if (code == VERSION_NOT_SUPPORTED) return "HTTP version not supported";
	return "Error";
}