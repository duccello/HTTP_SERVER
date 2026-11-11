#include "Config_Structs.hpp"

ServerConfig::ServerConfig()
	: root ("./www"),
	clientMaxBodySize(1024 * 1024),
	errorLog("error.log"),
	errorPageDir("./errors"),
	timeout(TIMEOUT) { }

LocationConfig::LocationConfig()
	: clientMaxBodySize(0),
	  autoidx(false),
	  returnCode(0) { }

ListenDirective::ListenDirective()
	: interface("0.0.0.0"),
	  port (-1) { }