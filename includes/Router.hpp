#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include <string>
#include <map>
#include <vector>
#include "Config_Structs.hpp"

class Router {
	public:
		int	 		handleRequest(const std::vector<ServerConfig>& servers, Client& client);
		void		checkBodySize(const std::vector<ServerConfig>& servers, Client& client);
		void		handleExpect(Client& client);
		const ServerConfig* 	findServer(const std::vector<ServerConfig>& servers, const Client& client);
		
	private:
		void			handleGet(const LocationConfig& loc, const std::string& fullPath, Client& client);
		void 			handlePost(const LocationConfig& loc, std::string fullPath, Client& client);
		void			handleDelete(const std::string& fullPath, Client& client);
		void			handleAutoindex(const std::string& fullPath, Client& client);
		int				handleCgi(Client& client, const std::string& fullPath, const LocationConfig& loc);
		int				_handleRedirection(Client& client, const LocationConfig& loc);
		bool					isCgi(const LocationConfig& loc, const std::string& path);
		bool    				validateMethod(const LocationConfig* loc, std::string method, const std::string& path);
		std::string 			buildFullPath(const std::string& path, const ServerConfig& server, const LocationConfig& loc) const;
		const LocationConfig* 	findLocation(const std::vector<ServerConfig>& servers, const Client& client, const std::string& path);
		const LocationConfig*	matchLocation(const ServerConfig& server, const std::string& path);
};

#endif