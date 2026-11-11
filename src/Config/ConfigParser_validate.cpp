#include "ConfigParser.hpp"
#include "utils.hpp"

#include <sstream>
#include <iostream>
#include <sys/stat.h>


/*
	Avoid duplicated ports between servers.
	- Iterates all servers and their port vectors
	- seenPorts accumulates all ports seen so far (local, destroyed on return)

	std::set::insert returns a std::pair<iterator, bool> -> !.second == duplicated
	oss usefull to print error message
*/
void ConfigParser::crossValidation(const std::vector<ServerConfig>& servers) {
	std::set<int> seenPorts;

	for (size_t i = 0; i < servers.size(); i++) {

		for (size_t j = 0; j < servers[i].ports.size(); j++) {

			int port = servers[i].ports[j].port;
			if (!seenPorts.insert(port).second){

				std::ostringstream oss;
				oss << port;
				throw std::runtime_error(errorPrefix(_line) + 
                    "Duplicate port across servers: " + oss.str() + RESET);
			}
		}
	}
}

/*
	To avoid duplicated location in the same server
	- Loop between the locations to check if there is any duplicated or not
*/
void ConfigParser::checkLocationDuplication(const ServerConfig& serv) {

	std::set<std::string> seenPaths;

	for (size_t i = 0; i < serv.locations.size(); i++) {
		if (!seenPaths.insert(serv.locations[i].path).second) {
			throw std::runtime_error(errorPrefix(_line) 
				+ "Duplicate location path: " + serv.locations[i].path + RESET);
		}
	}
}

/*
	- It should have at least 1 listen
	- no duplicated ports
	- not empty root
*/
void ConfigParser::validateServer(const ServerConfig& serv) {

	if (serv.ports.empty())
		throw std::runtime_error(errorPrefix(_line) + "Server has no listen directive" + RESET);
	
	// for (size_t i = 0; i < serv.ports.size(); i++) {
	// 	if (serv.ports[i].port == -1)
	// 		throw std::runtime_error(errorPrefix(_line) + "missing port" + RESET);
	// }

	if (serv.timeout <= 0)
		throw std::runtime_error(errorPrefix(_line) + "Timeout must be greater than 0" + RESET);

	if (serv.clientMaxBodySize == 0) // NGINX allows it and deactivate the MaxBodySize
	    throw std::runtime_error(errorPrefix(_line) + "client_max_body_size must be greater than 0" + RESET);

	for (size_t i = 0; i < serv.ports.size(); i++) {
		for (size_t j = i + 1; j < serv.ports.size(); j++) {
			if (serv.ports[i].port == serv.ports[j].port) {

				std::ostringstream oss; // to write/convert int to str
				oss << serv.ports[i].port;
				throw std::runtime_error(errorPrefix(_line) + "Duplicate port: " + oss.str() + RESET);
			}
		}
	}
	// Check to avoid duplicated paths within the server
	checkLocationDuplication(serv);
}

/*
	- It should allow cases like: `www.example.com`
	- This is checked with different loops on the each string name
*/
bool ConfigParser::isValidServerName(const std::string& name) {

	// handles if name.size() is different than `*`, like `a`
	if (name.size() < 2) 
		return isalnum(name[0]);

	// Not allow to start or end with `.` or `-`
	char first = name[0] == '*' ? name[2] : name[0];
	char last = name[name.size()-1] == '*' ? name[name.size()-3] : name[name.size()-1];
	if (first == '-' || first == '.' || last == '-' || last == '.') 
		return false;

	// Not allow 2 consecutive dots
	for (size_t i = 0; i < name.size() - 1; i++) {
		if (name[i] == '.' && name[i+1] == '.') {
		return false;}
	}

	// only digits, letteres, `-` or `.` between the words.
	for (size_t i = 0; i < name.size(); i++) {
		char c = name[i];
		if (!isalnum(c) && c != '-' && c != '.'){
			return false;
		}
	}
	return true;
}

/*
	- It's not empty
	- path starts with /
	- all methods are valid
	- no other loc directive when there is a returnCode (redirect is exclusive)

*/
// Validate the final state of the object
void ConfigParser::validateLocation(const LocationConfig& loc) { 

	// location path check
	if (loc.path.empty() || loc.path[0] != '/')
		throw std::runtime_error(errorPrefix(_line) + "Location path must start with /" + RESET);

	// method validation
	for (size_t i = 0; i < loc.methods.size(); i++) {
		if (!isValidMethod(loc.methods[i]))
			throw std::runtime_error(errorPrefix(_line) + "Invalid method: " + loc.methods[i] + RESET);
	}

	// exclusive directive for redirections: nos mtd or root
	if (loc.returnCode != 0 && (!loc.methods.empty() || !loc.root.empty()))
		throw std::runtime_error(errorPrefix(_line) + "Return directive is exclusive" + RESET);
	
	// CGI warning
	for (std::map<std::string, std::string>::const_iterator it = loc.cgiHandlers.begin();
		 it != loc.cgiHandlers.end(); ++it) {
		struct stat st;
		if (stat(it->second.c_str(), &st) != 0) {
			std::cerr << WARNING << "[Warning] line: " << _line << "CGI binary not found: " << it->second << RESET << std::endl;
		}
	}

	// Check CGI with GET or POST
	if (!loc.cgiHandlers.empty()) {
		bool hasGetOrPost = false;
		for (size_t i = 0; i < loc.methods.size(); i++) {
			if (loc.methods[i] == "GET" || loc.methods[i] == "POST") { hasGetOrPost = true; break; }
		}
		if (!hasGetOrPost)
			throw std::runtime_error(errorPrefix(_line) + "GET or POST required for CGI" + RESET);
	}
}

// Needed to have a minimal config able to work
void ConfigParser::defaultLocation(ServerConfig& serv) {

	if (serv.root.empty()) {
		serv.root = "www";
	}

	bool hasRoot = false;

	// check if there is any location
	for (size_t i = 0; i < serv.locations.size(); i++) {

		if (serv.locations[i].clientMaxBodySize == 0)
			serv.locations[i].clientMaxBodySize = serv.clientMaxBodySize;

		if (serv.locations[i].path == "/") {
				hasRoot = true;
				if (serv.locations[i].idx.empty()) {
					// inject default index if missing
					serv.locations[i].idx = "index.html";}
			}
		}
	
	if (!hasRoot) {
		LocationConfig loc;
		loc.path = "/";
		loc.root = serv.root;
		loc.methods.push_back("GET");
		loc.idx = "index.html";
		serv.locations.push_back(loc);
	}
}

/*
	ports 0-1023 reserved (IANA). eg: HTTP=80, HTTPS=443, SSH=22
	65535 is the TCP/IP limit == 2^16 - 1 (2 bytes)
*/
bool ConfigParser::isValidPort(int port) {
	return (port > 1023 && port <= 65535);
}

bool ConfigParser::isValidMethod(const std::string& mtd) {
	return (mtd == "GET" || mtd == "POST" || mtd == "DELETE");
}

bool ConfigParser::isValidIP(const std::string& ip) {
	int count = 0;
	std::istringstream iss(ip);
	std::string octet;

	while (std::getline(iss, octet, '.')) {

		if (octet.empty() || octet.size() > 3)
			return false;

		if (octet.size() > 1 && octet[0] == '0')
			return false;

		for (size_t i = 0; i < octet.size(); i++) {
			if (!isdigit(octet[i]))
				return false;
		}

		std::istringstream iss2(octet);
		int value;
		if (!(iss2 >> value))
    		return false;

		if (value < 0 || value > 255)
			return false;
		count++;
	}

	if (count == 4)
		return true;
	else
		return false;
}

/*
	ej "1m" -> 1048576 (convert to int)
	Base factor: 1 (1024 * 1024 = 1m)
*/

size_t ConfigParser::parseBodySize (const std::string& val) {

	size_t	mult = 1;
	std::string nb = val;

	if (!val.empty()) {

		char last = val[val.size() - 1];
		if (last == 'm' || last == 'M') {
			mult = 1024 * 1024;
			nb = val.substr(0, val.size() -1);
		}

		else if (last == 'k' || last == 'K') {
			mult = 1024;
			nb = val.substr(0, val.size() -1);
		}
	}

	if (!nb.empty() && nb[0] == '-')
		throw std::runtime_error(errorPrefix(_line) + "Invalid body size: " + val + RESET);

	std::istringstream iss(nb); // to read/convert str to type
	size_t size;

	//check that the number is valid
	if (!(iss >> size)){
		throw std::runtime_error(errorPrefix(_line) + "Invalid body size: " + val + RESET);
	}

	//check there is not rubish leftovers
	std::string leftovers;
    if (iss >> leftovers) {
        throw std::runtime_error(errorPrefix(_line) + "Invalid body size characters: "+ val + RESET);
    }

	// INT MAX
	if (size > std::numeric_limits<size_t>::max() / mult) {
        throw std::runtime_error(errorPrefix(_line) + "client_max_body_size exceeds maximum memory limit" + RESET);
    }
	return size * mult;
}