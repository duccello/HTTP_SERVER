#include "Webserv.hpp"
#include "Request.hpp"

#include <cstdlib>

bool Request::areHeadersComplete(const std::string& buffer)
{
    if (buffer.find("\r\n\r\n") == std::string::npos) 
        return false;
	else
		return true;
}

bool Request::isBodyComplete(const std::string& buffer)
{
	size_t	header_end = buffer.find("\r\n\r\n");
	//checking for a the content length- if none, there will be no body.
    size_t content_length_pos = buffer.find("Content-Length: ");
    if (content_length_pos == std::string::npos)
	{
		this->_size_incl_header = header_end + 4;
	    return true;
	}

    size_t value_start = content_length_pos + 16;
	size_t content_length = std::strtoul(buffer.c_str() + value_start, NULL, 10);
    size_t body_start = header_end + 4;
    size_t body_received = buffer.size() - body_start;
	// checking that the size of the body is actually matching the content length, 
	// otherwise we need to recv() more
    if (body_received >= content_length)
	{
		this->_size_incl_header = header_end + 4 + content_length;
        return true;
	}
    return false;
}

void	Request::parseHeaders(const std::string& buffer)
{
	//extracting method
	size_t method_end = buffer.find(" ");
    if (method_end == std::string::npos)
		throw HttpException(BAD_REQUEST, "bad request: no method");
	this->_method = buffer.substr(0, method_end);
	
	//extracting target
    size_t target_end = buffer.find(" ", method_end + 1);
    if (target_end == std::string::npos)
		throw HttpException(BAD_REQUEST, "bad request: no target");
	this->_target = buffer.substr(method_end + 1, target_end - method_end - 1);
	if (_target.find("..") != std::string::npos)
	    throw HttpException(FORBIDDEN, "bad request: client trying to acess backward directories");
	
	//query_string
	size_t query_start = _target.find("?");
	if (query_start != std::string::npos)
	{
		_query_string = _target.substr(query_start + 1);
		_target = _target.substr(0, query_start);
	}
	//extracting protocol
    size_t protocol_end = buffer.find("\r\n", target_end + 1);
    if (protocol_end == std::string::npos)
		throw HttpException(BAD_REQUEST, "bad request: no protocol");
	this->_protocol = buffer.substr(target_end + 1, protocol_end - target_end - 1); 

	//extracting headers
	size_t header_pos = protocol_end + 2;
	size_t header_end = buffer.find("\r\n\r\n");
	while (header_pos < header_end)
	{
		size_t line_end = buffer.find("\r\n", header_pos);
		std::string line = buffer.substr(header_pos, line_end - header_pos);
		
		size_t key_end = line.find(": ");
		if (key_end != std::string::npos)
		{
			std::string key = line.substr(0, key_end);
			std::string value = line.substr(key_end + 2);
			this->_headers[key] = value;
		}
		header_pos = line_end + 2;
	}
}

void	Request::parseBody(const std::string& buffer)
{
	size_t header_end = buffer.find("\r\n\r\n");
	if (this->_headers.count("Content-Length"))
	{
		size_t content_length = std::strtoul(this->_headers["Content-Length"].c_str(), NULL, 10);	
		this->_body = buffer.substr(header_end + 4, content_length);
	}
}

void Request::isValid() const 
{
    if (_protocol != "HTTP/1.1")
        throw HttpException(VERSION_NOT_SUPPORTED, "HTTP version not supported");
    if (_target.empty() || _target[0] != '/')
        throw HttpException(BAD_REQUEST, "bad request: invalid target");
    if (_headers.find("Host") == _headers.end())
        throw HttpException(BAD_REQUEST, "bad request: missing Host header");
    if (_method != "GET" && _method != "POST" && _method != "DELETE") 
		throw HttpException(NOT_ALLOWED, "bad request: unsupported method");
	if (_method == "GET" && _headers.find("Content-Length") != _headers.end())
        throw HttpException(BAD_REQUEST, "bad request: invalid GET");
}
