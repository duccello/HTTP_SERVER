
#ifndef CONFIG_STRUCTS_HPP
# define CONFIG_STRUCTS_HPP

# include <vector>
# include <iostream>
# include <map>

#define TIMEOUT 10

struct LocationConfig;
struct ListenDirective;

struct ServerConfig 
{
	ServerConfig();

	std::vector<ListenDirective> ports;
	
	std::string serverName;
	std::string root;
	std::vector<LocationConfig> locations;

	size_t clientMaxBodySize; 
	
	std::string accessLog;
	std::string errorLog;
	std::string errorPageDir;

	int timeout;

	std::map<std::string, std::string> mimeType;
	std::map<int, std::string> error_pages;
};

struct LocationConfig
{
	LocationConfig();

	std::string path;
	std::string root;
	std::vector <std::string> methods;

	size_t clientMaxBodySize;

	std::string idx;//default
	bool autoidx;//activate or not listing

	// CGI
	std::map<std::string, std::string> cgiHandlers; 

	// Redirect
	int returnCode;
	std::string returnPath;
};

struct ListenDirective
{
	ListenDirective();

	std::string interface;
	int         port;
};

#endif