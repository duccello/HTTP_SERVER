#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include "Config_Structs.hpp"

#define CONTINUE 100
#define OK 200
#define CREATED 201
#define ACCEPTED 202
#define NO_CONTENT 204
#define MOVED_PERMANENTLY 301
#define FOUND 302
#define SEE_OTHER 303
#define TEMPORARY_REDIRECT 307
#define PERMANENT_REDIRECT 308
#define BAD_REQUEST 400
#define FORBIDDEN 403
#define NOT_FOUND 404
#define NOT_ALLOWED 405
#define NOT_ACCEPTABLE 406
#define REQUEST_TIMEOUT 408
#define	CONFLICT 409
#define LENGTH_REQUIRED 411
#define TOO_LARGE 413
#define UNSUPPORTED 415
#define SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define BAD_GATEWAY 502
#define GATEWAY_TIMEOUT 504
#define VERSION_NOT_SUPPORTED 505 
class Client;

class	HttpException : public std::exception
{
	public:
	HttpException(int status, const std::string& msg) : _status(status), _msg(msg) {}
	const char* what() const throw() { return _msg.c_str(); }
	int getStatus() const { return _status; }

    ~HttpException() throw() {}

	private:
	int	_status;
	std::string _msg;
};

class Response 
{
	private:
		int									_status;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		unsigned long						_bytes_sent;
		bool								_response_to_send;

	public:
		// Canonical
		Response(void);
		~Response(void);
		Response(const Response& src);
		Response& operator=(const Response& src);

		// Getters
		int											getStatus() const;
		const unsigned long&						getBytesSent() const;
		const std::string&							getBody() const;
		const std::map<std::string, std::string>&	getHead() const;
		static std::string							getStatusMessage(int code);
		const bool&									getResponseToSend() const; //no need of int& because that's cheap to use a copy
		// Setters
		void	setStatus(int status);
		void	setBody(const std::string& body);
		void	setHead(const std::string& key, const std::string& value);
		void	setBytesSent(unsigned long bytes_sent);
		void	setResponseToSend(bool response_ready);

		//Methods
		std::string toString() const;
		void		buildError(int code, const ServerConfig& serv, Client& client);
		std::string	getStatusMsg(int code) const;

};

#endif