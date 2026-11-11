#include "Request.hpp"

Request::Request()
{
	_headers_parsed	= false;
	_size_incl_header = 0;
}

Request::Request(const Request &other)
{
    *this = other;
}

Request &Request::operator=(const Request &other)
{
    if (this != &other)
    {
		_headers_parsed	= other._headers_parsed;
		_method   = other._method;
        _target   = other._target;
        _protocol = other._protocol;
        _headers  = other._headers;
        _body     = other._body;
		_size_incl_header = other._size_incl_header;
    }
    return *this;
}

Request::~Request()
{
}

// Getters
const std::string& Request::getMethod() const {
	return _method;
}

const std::string& Request::getTarget() const {
	return _target;
}

const std::string& Request::getQueryString() const {
	return _query_string;
}

const std::map<std::string, std::string>& Request::getHeaders() const {
	return _headers;
}

const std::string& Request::getBody() const {
	return _body;
}

//we need this as a mandatory field
const std::string& Request::getProtocol() const {
	return _protocol;
}

const size_t& Request::getSizeInclHeader() const {
	return _size_incl_header;
}

// Setters
void Request::setMethod(const std::string& mtd) {
	_method = mtd;
}

void Request::setTarget(const std::string& target) {
	_target = target;
}

void Request::setProtocol(const std::string& vs) {
	_protocol = vs;
}

void Request::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void Request::setBody(const std::string& body) {
	_body = body;
}