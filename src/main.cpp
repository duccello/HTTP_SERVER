#include "Webserv.hpp"
#include "ConfigParser.hpp"

void ft_debug_print(std::vector<ServerConfig> servers)
{
		std::cout << "Parsed " << servers.size() << " server(s)\n";
	for (size_t i = 0; i < servers.size(); i++) 
	{//	TEST
		std::cout << "Server " << i << " ports: ";
		for (size_t j = 0; j < servers[i].ports.size(); ++j)
		{
			std::cout << servers[i].ports[j].port;
			if (j + 1 < servers[i].ports.size())
				std::cout << ", ";
		}
		std::cout << " root: " << servers[i].root
          << " locations: " << servers[i].locations.size()
          << "\n";
	}
}

int main (int argc, char** argv)
{

	if (argc != 2)
	{
		std::cerr << "Program needs a single parameter (config filename)\n";
		return (1);
	}
	try
	{
		ConfigParser config;
		Webserv&  webserv = Webserv::getInstance();
		webserv.setServers(config.parse(argv[1]));
		ft_debug_print(webserv.getServers());
		std::signal(SIGINT, Webserv::handleSigint);
		if (webserv.startServer() == 1)
			return (1);
	}
	catch (std::exception& e) {
			std::cerr << "Error: " << e.what() << "\n";
	}
	return (0);
}