#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ConfigTokenizer.hpp"
#include "Webserv.hpp"

#include <set>
#include <fstream>

enum ServerDirective {
	SD_LISTEN,
	SD_SERVER_NAME,
	SD_ROOT,
	SD_CLIENT_MAX_BODY_SIZE,
	SD_ERROR_LOG,
	SD_ERROR_PAGE,
	SD_ERROR_DIR,
	SD_TIMEOUT,
	SD_MIME_TYPE,
	SD_LOCATION,
	SD_UNKNOWN
};

enum LocationDirective {
	LD_METHODS,
	LD_ROOT,
	LD_CLIENT_MAX_BODY_SIZE,
	LD_INDEX,
	LD_AUTOINDEX,
	LD_CGI_EXT,
	LD_RETURN,
	LD_UNKNOWN
};

// default constructor set but the compiler
class ConfigParser {
	private:

		// Attributes
		std::vector<std::pair<std::string, int> > _tokens;
		size_t _pos;
		int	_line;
		std::string _currentDirective;
		template <typename T>

		// OCF -> to avoid having 2 parsers
		ConfigParser();
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);

		// General helpers
		std::string nextToken();
		void checkToken(const std::string& token);

		// Parser
		ServerConfig parseServer();
		LocationConfig parseLocation();

		// Directives parser - server
		void parseServerDirective(const std::string& token, ServerConfig& serv);
		ServerDirective toServerDirective(const std::string& token);

		void parseListen(ServerConfig& serv);
		void parseServerName(ServerConfig& serv);
		void parseRoot(ServerConfig& serv);
		void ParseClientMaxBoSi(ServerConfig& serv);
		void parseErrorLog(ServerConfig& serv);
		void parseErrorDir(ServerConfig& serv);
		void parseErrorPage(ServerConfig& serv);
		void parseTimeOut(ServerConfig& serv);
		void parseMimeType(ServerConfig& serv);

		// Directives parser - location
		void parseLocationDirective(const std::string& token, LocationConfig& loc);
		LocationDirective toLocationDirective(const std::string& token);
		void parseMethods(LocationConfig& loc);
		void parseRootLoc(LocationConfig& loc);
		void parseClientMaxBoSiLoc(LocationConfig& loc);
		void parseIndex(LocationConfig& loc);
		void parseAutoindex(LocationConfig& loc);
		void parseCgiExt(LocationConfig& loc);
		void parseReturn(LocationConfig& loc);

		// Validation -> semantic, post parser (doesn't modify like parser)
		void crossValidation(const std::vector<ServerConfig>& servers);
		void validateServer(const ServerConfig& serv);
		void validateLocation(const LocationConfig& loc);
		void defaultLocation(ServerConfig& serv);

		// Validation Helpers
		bool isValidPort(int port);
		bool isValidMethod(const std::string& mtd);
		bool isValidServerName(const std::string& name);
		void checkLocationDuplication(const ServerConfig& serv);
		size_t parseBodySize (const std::string& val); // ej "1m" -> 1048576 (convert to int)
		std::string checkDirective();
		bool isValidIP(const std::string& ip);

		// Template
		template <typename T> T convertToNumber(const std::string& token,
							const std::string& directiveName);

	public:

		//Constructor with param
		ConfigParser(void);

		// Public methods
		std::vector<ServerConfig> parse(const std::string& path); // with all config servers
};

// Template implementation
#include "ConfigParser.tpp"

#endif