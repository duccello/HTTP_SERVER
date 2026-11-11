#include "Response.hpp"
#include "Client.hpp"
#include "utils.hpp" 

#include <sstream> 

Response::Response(void) : _status(0), _bytes_sent(0), _response_to_send(false)
{
}

Response::~Response(void)
{
}

Response::Response(const Response& src)
{
	*this = src;
}

Response& Response::operator=(const Response& src)
{
	if (this != &src)
	{
		this->_status = src._status;
		this->_headers = src._headers;
		this->_body = src._body;
		this->_bytes_sent = src._bytes_sent;
		this->_response_to_send = src._response_to_send;
	}
	return *this;
}

int Response::getStatus() const {
	return _status;
}

const unsigned long& Response::getBytesSent() const {
	return _bytes_sent;
}

const std::string& Response::getBody() const {
	return _body;
}

const std::map<std::string, std::string>& Response::getHead() const {
	return _headers;
}

const bool& Response::getResponseToSend() const
{
	return _response_to_send;
}

void Response::setStatus(int status) {
	_status = status;
}

void Response::setBytesSent(unsigned long bytes_sent) {
	_bytes_sent = bytes_sent;
}

void Response::setBody(const std::string& body) {
	_body = body;
}

void Response::setHead(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void Response::setResponseToSend(bool response_ready)
{
	_response_to_send = response_ready;
}
