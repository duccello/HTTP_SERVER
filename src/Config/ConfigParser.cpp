#include "ConfigParser.hpp"
#include "Webserv.hpp"
#include "utils.hpp"


// ----------- //
// Constructor //
// ----------- //

// get the tokens to parse them
ConfigParser::ConfigParser(void) 
	: _pos(0),
	 _line(0) {
}

// -------------- //
// Public methods //
// -------------- //


// get servers parsed
std::vector<ServerConfig> ConfigParser::parse(const std::string& path) {

	ConfigTokenizer tokenizer(path);
	_tokens = tokenizer.getTokens();
	std::vector<ServerConfig> servers;

	//for (size_t i = 0; i < _tokens.size(); i++) {
	//	std::cout << "Token[" << i << "] Line " << _tokens[i].second << ": \"" << _tokens[i].first << "\"" << std::endl;
	//} //DEBUG
	while (_pos < _tokens.size()) {

		std::string token = nextToken();
		if (token == "server")
			servers.push_back(parseServer());
		else
			throw std::runtime_error(errorPrefix(_line) + "Expected 'server', got: "+ token + RESET);
	}
	if (servers.empty()){
		throw std::runtime_error(errorPrefix(_line) + "Config file has no server blocks" + RESET);
	}
	// Cross-server ports
	crossValidation(servers);

	return servers;
}

// Helpers
std::string ConfigParser::checkDirective() {
    std::string token = nextToken();
    if (token == ";") {
        throw std::runtime_error(errorPrefix(_line) + "Empty directive value for '" + _currentDirective + "'" + RESET);
    }
    return token;
}

std::string ConfigParser::nextToken() {
	if (_pos >= _tokens.size()){
		throw std::runtime_error(errorPrefix(_line) + "Unexpected end of config" + RESET); }
	_line = _tokens[_pos].second;
	return _tokens[_pos++].first;
}


void ConfigParser::checkToken(const std::string& expected) {
	std::string token = nextToken();
	if (token != expected)
		throw std::runtime_error(errorPrefix(_line) + "ConfigParser: Expected '" + expected + "' but got '" + token + "'" + RESET);
}