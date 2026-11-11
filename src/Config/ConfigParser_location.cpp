#include "ConfigParser.hpp"
#include "utils.hpp"

#include <sstream>
#include <iostream>
#include <set>

// --------------- //
// Private methods //
// --------------- //

void ConfigParser::parseMethods(LocationConfig& loc){

	std::string token;
	while ((token = nextToken()) != ";") {
		if (!isValidMethod(token))
			throw std::runtime_error(errorPrefix(_line) + "Invalid method: " + token + RESET);
		loc.methods.push_back(token);
	}
	if (loc.methods.empty()) {
		throw std::runtime_error(errorPrefix(_line) + "Empty directive value for '" + _currentDirective + "'" + RESET);
	}
}

void ConfigParser::parseRootLoc(LocationConfig& loc) 
{
	loc.root = checkDirective();
	checkToken(";");
}

void ConfigParser::parseClientMaxBoSiLoc(LocationConfig& loc){

	std::string token = checkDirective();

	loc.clientMaxBodySize = parseBodySize(token);

	if (loc.clientMaxBodySize == 0) {
		throw std::runtime_error(std::string(FATAL_ERROR) + "client_max_body_size must be greater than 0" + RESET);
	}
	
	checkToken(";");
}

// Allows multiple index files per server
void ConfigParser::parseIndex(LocationConfig& loc){

	std::string token = checkDirective();
	loc.idx = token;
	checkToken(";");
}

void ConfigParser::parseAutoindex(LocationConfig& loc){
	std::string token = checkDirective();

	if (token != "on" && token != "off" && token != "true" && token != "false")
		throw std::runtime_error(errorPrefix(_line) + "Invalid autoindex value: " + token + RESET);
	loc.autoidx = (token == "on" || token == "true");
	checkToken(";");
}

void ConfigParser::parseCgiExt(LocationConfig& loc){
	std::string ext = checkDirective(); // eg: ".py"
	std::string binary = nextToken(); // eg: "/usr/bin/python3"

	if (binary == ";")
		throw std::runtime_error(errorPrefix(_line) + "Incomplete arguments for cgi_ext: '" + ext + "'" + RESET);

	if (ext.empty() || ext[0] != '.') {
		throw std::runtime_error(errorPrefix(_line) + "cgi_ext extension must start with '.': " + ext + RESET);
	}

	if (ext.size() < 2) {
		throw std::runtime_error(errorPrefix(_line) + "incomplete cgi_ext extension: '" + ext + "'" + RESET);
	}

	if (loc.cgiHandlers.count(ext))
		std::cerr << WARNING << "[Warning] line " << _line << ": cgi_ext '" << ext << "' redefined\n" << RESET;

	loc.cgiHandlers[ext] = binary;
	checkToken(";");
}

// redir example: return 301 /new-path;
void ConfigParser::parseReturn(LocationConfig& loc){

	std::string codeStr = checkDirective();
	std::istringstream iss(codeStr);

	// check is a valid number
	if(!(iss >> loc.returnCode)){
		throw std::runtime_error(errorPrefix(_line) + "Invalid return code: " + codeStr + RESET);
	}

	std::string leftover;
	if (iss >> leftover)
		throw std::runtime_error(errorPrefix(_line) + "Invalid code: " + codeStr + RESET);

	// check range
	if (loc.returnCode < 300 || loc.returnCode > 399) {
		throw std::runtime_error(errorPrefix(_line) + "Invalid return code '" + codeStr + "', must be between 300 and 399" + RESET);
	}

	// read path
	//loc.returnPath = checkDirective();
	loc.returnPath = nextToken();

	// check valid path
	if (loc.returnPath == ";") {
		throw std::runtime_error(errorPrefix(_line) + "invalid number of arguments in 'return' directive: " + codeStr + RESET);
	}
	checkToken(";");
}

LocationDirective ConfigParser::toLocationDirective(const std::string& token){
	if (token == "method" || token == "methods" || token == "allowed_methods")	 return LD_METHODS;
	if (token == "root")														 return LD_ROOT;
	if (token == "client_max_body_size")										 return LD_CLIENT_MAX_BODY_SIZE;
	if (token == "index")														 return LD_INDEX;
	if (token == "autoindex" || token == "autoidx")								 return LD_AUTOINDEX;
	if (token == "cgi_ext")														 return LD_CGI_EXT;
	if (token == "return")														 return LD_RETURN;
	return LD_UNKNOWN;
}

// To avoid if else per each key word in the config
void ConfigParser::parseLocationDirective(const std::string& token, LocationConfig& loc) {

	_currentDirective = token;

	switch (toLocationDirective(token)) {
		case LD_METHODS: parseMethods(loc);
			break;
		case LD_ROOT: parseRootLoc(loc);
			break;
		case LD_CLIENT_MAX_BODY_SIZE: parseClientMaxBoSiLoc(loc);
			break;
		case LD_INDEX: parseIndex(loc);
			break;
		case LD_AUTOINDEX: parseAutoindex(loc);
			break;
		case LD_CGI_EXT: parseCgiExt(loc);
			break;
		case LD_RETURN: parseReturn(loc);
			break;
		
		default: throw std::runtime_error(errorPrefix(_line) + "Unknown location directive: " + token + RESET); // LD_UNKNOWN no used?
	}
}

/* 1. Route standardization:
		1.1. Find last position that is not `/`
		1.2. Get the substr from position 0 until the last not slash(path wihtout the last `/`)
		It standarinze cases like: ej: "////" to `/`
	2. Allow specific directives to be duplicated
*/
LocationConfig ConfigParser::parseLocation() {

	LocationConfig loc;
	std::set<std::string> seenDir;

	loc.path = nextToken();// read "/upload", "/cgi", etc

	size_t last_not_slash = loc.path.find_last_not_of('/');

	if (last_not_slash != std::string::npos) {
		loc.path = loc.path.substr(0, last_not_slash + 1);
	} else {
		loc.path = "/";
	}

	checkToken("{");

	std::string token;
	while ((token = nextToken()) != "}"){
		bool allowDup = (token == "cgi_ext");

		if (!allowDup) {
			if (!seenDir.insert(token).second)
				throw std::runtime_error(errorPrefix(_line) + "Duplicate directive: " + token + RESET);
		}
		parseLocationDirective(token, loc);}
	
	validateLocation(loc);

	return loc;
}
