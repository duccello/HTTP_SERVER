#include "ConfigParser.hpp"
#include "utils.hpp"

#include <sstream>
#include <limits>
#include <set>

/*
	Directive examples:
			listen 127.0.0.1:8080;
			listen 8080;
*/
void ConfigParser::parseListen(ServerConfig& serv) {

	std::string token = checkDirective();
	ListenDirective ld;

	size_t colon = token.find(':');

	if (colon != std::string::npos) {
		std::string interface = token.substr(0, colon);
		std::string portStr = token.substr(colon + 1);

		if (!isValidIP(interface))
				throw std::runtime_error(errorPrefix(_line) + "Invalid interface: " + interface + RESET);
		
		ld.interface = interface;
		ld.port = convertToNumber<int>(portStr, "port");
	} else {
		ld.port = convertToNumber<int>(token, "port");
	}

	serv.ports.push_back(ld);
	checkToken(";");
}

void ConfigParser::parseRoot(ServerConfig& serv) {

	serv.root = checkDirective();
	checkToken(";");
}

// accept m and k or only numbers
void ConfigParser::ParseClientMaxBoSi(ServerConfig& serv){

	std::string token = checkDirective();

	serv.clientMaxBodySize = parseBodySize(token);

	if (serv.clientMaxBodySize == 0) {
        throw std::runtime_error(errorPrefix(_line) + "client_max_body_size must be greater than 0" + RESET);
    }
	
	checkToken(";");
}

void ConfigParser::parseErrorLog(ServerConfig& serv) {

	std::string token = checkDirective();

	serv.errorLog = token;
	checkToken(";");
}

void ConfigParser::parseErrorDir(ServerConfig& serv) {
	serv.errorPageDir = checkDirective();
	checkToken(";");
}

void ConfigParser::parseErrorPage(ServerConfig& serv) {

	std::string codeStr = checkDirective();
	//std::string path = checkDirective();
	std::string path = nextToken();
	int code;
	std::istringstream iss (codeStr);

	if (!(iss >> code))
		throw std::runtime_error(errorPrefix(_line) + "Invalid error code" + RESET);

	std::string leftover;
	if (iss >> leftover)
		throw std::runtime_error(errorPrefix(_line) + "Invalid code: " + codeStr + RESET);

	if (code < 400 || code > 599)
		throw std::runtime_error(errorPrefix(_line) + "error_page code must be 4xx or 5xx" + RESET);

	if (path == ";")
		throw std::runtime_error(errorPrefix(_line) + "invalid number of arguments in error_page: " + codeStr + RESET);

	if (serv.error_pages.count(code))
		std::cerr << WARNING << "[Warning] line " << _line << ": error_page " << code << " redefined" << RESET << std::endl;

	serv.error_pages[code] = path;
	checkToken(";");
}

// iss is interpreted as int because timeout defition & >> overload solves it as type-safe
void ConfigParser::parseTimeOut(ServerConfig& serv) {

	std::string token = checkDirective();
	serv.timeout = convertToNumber<int>(token, "timeout");
	checkToken(";");
}

/*
	Example:
	config:    mime_type .html text/html;
	tokens:    [".html"] ["text/html"] [";"]
	nextToken() → ext  = ".html"
	nextToken() → type = "text/html"
	serv.mimeType[".html"] = "text/html"
	expect(";") → consume ";"
*/
void ConfigParser::parseMimeType(ServerConfig& serv) {

	std::string ext = checkDirective();
	std::string type =  nextToken();

	if (type == ";")
		throw std::runtime_error(errorPrefix(_line) + "invalid number of arguments in mime_type: " + ext + RESET);

	if (ext.empty() || ext[0] != '.')
		throw std::runtime_error(errorPrefix(_line) + "mime_type extension must start with '.': " + ext + RESET);

	if (ext[0] == '.' && ext.size() < 2)
		throw std::runtime_error(errorPrefix(_line) + "incomplete mime_type extension: '" + ext + "'" + RESET);//NEW

	if (serv.mimeType.count(ext))
			if (serv.mimeType.count(ext))
			std::cerr << WARNING << "[Warning] line " << _line << ": mimetype " << ext << " redefined" << RESET << std::endl;
	serv.mimeType[ext] = type;

	checkToken(";");
}

void ConfigParser::parseServerName(ServerConfig& serv) {

	std::string token = nextToken();

	if (token == ";") {
		throw std::runtime_error(errorPrefix(_line) + "Empty directive value for '" + _currentDirective + "'" + RESET);
	}

	if (!isValidServerName(token)) {
		throw std::runtime_error(errorPrefix(_line) + "Invalid server_name syntax: '" + token + "'" + RESET);
	}

	serv.serverName = token;
	checkToken(";");
}

ServerDirective ConfigParser::toServerDirective(const std::string& token) {
	if (token == "listen" || token == "server_port") return SD_LISTEN;
	if (token == "server_name")						 return SD_SERVER_NAME;
	if (token == "root")							 return SD_ROOT;
	if (token == "client_max_body_size")			 return SD_CLIENT_MAX_BODY_SIZE;
	if (token == "error_log")						 return SD_ERROR_LOG;
	if (token == "error_page")						 return SD_ERROR_PAGE;
	if (token == "error_page_dir")					 return SD_ERROR_DIR;
	if (token == "timeout")							 return SD_TIMEOUT;
	if (token == "mime_type")						 return SD_MIME_TYPE;
	if (token == "location")						 return SD_LOCATION;
	return SD_UNKNOWN;
}

// To avoid if else per each key word in the config
void ConfigParser::parseServerDirective(const std::string& token, ServerConfig& serv) {

	_currentDirective = token;

	switch (toServerDirective(token))
	{
		case SD_LISTEN: parseListen(serv);
			break;
		case SD_SERVER_NAME: parseServerName(serv); 
			break;
		case SD_ROOT: parseRoot(serv);
			break;
		case SD_CLIENT_MAX_BODY_SIZE: ParseClientMaxBoSi(serv);
			break;
		case SD_ERROR_LOG: parseErrorLog(serv);
			break;
		case SD_ERROR_PAGE: parseErrorPage(serv);
			break;
		case SD_ERROR_DIR: parseErrorDir(serv);
			break;
		case SD_TIMEOUT: parseTimeOut(serv);
			break;
		case SD_MIME_TYPE: parseMimeType(serv);
			break;
		case SD_LOCATION: serv.locations.push_back(parseLocation());
			break;

		default: throw std::runtime_error(errorPrefix(_line) + "Unknown server directive: " + token + RESET);
	}
}

ServerConfig ConfigParser::parseServer() {

	ServerConfig serv;
	std::set<std::string> seenDir;

	checkToken("{");

	std::string token;
	while ((token = nextToken()) != "}") {

		bool allowDup = (token == "listen" || token == "port" || token == "listen port" || token == "error_page"
						|| token == "mime_type" || token == "location");
		
		if (!allowDup) {
			if (!seenDir.insert(token).second)
				throw std::runtime_error(errorPrefix(_line) + "Duplicate directive: " + token + RESET);
		}
		parseServerDirective(token, serv);
	}

	defaultLocation(serv);
	validateServer(serv);

	return serv;
}