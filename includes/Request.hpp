#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>

class Request
{
    private:
		std::string							_method;
        std::string 						_target;
		std::string 						_query_string;
		std::string 						_protocol;
		std::map<std::string, std::string>	_headers;
		std::string 						_body;
		size_t								_size_incl_header;

    public:
		Request();
		Request(const Request &other);
        Request &operator=(const Request &other);
        ~Request();

		const std::string& getMethod() const;
		const std::string& getTarget() const;
		const std::string& getQueryString() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
		const std::string& getProtocol() const;
		const size_t& getSizeInclHeader() const;

		// SETTERS
		void setMethod(const std::string& mtd);
		void setTarget(const std::string& Target);
		void setProtocol(const std::string& vs);
		void setHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);

		// Public methods
		void 		isValid() const;
		bool		areHeadersComplete(const std::string& buffer);
		void		parseHeaders(const std::string& buffer);
		bool		isBodyComplete(const std::string& buffer);
		void		parseBody(const std::string& buffer);
		
		// PUblic members
		bool _headers_parsed;
};

#endif
