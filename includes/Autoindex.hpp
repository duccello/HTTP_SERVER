#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

#include <dirent.h> //DIR*
#include <vector>
#include <string>

class Autoindex {
    private:

        //OCF
        Autoindex();
        Autoindex(const Autoindex&);
        Autoindex& operator=(const Autoindex);

        // methods
        static std::string htmlEscape(const std::string& s);
        static std::vector<std::string> listEntries(DIR* dir);
		static std::string renderEntry(const std::string& fullPath, const std::string& name);

    public:

        static std::string generateAutoidx(const std::string& fullPath, const std::string& uriPath);
};

#endif