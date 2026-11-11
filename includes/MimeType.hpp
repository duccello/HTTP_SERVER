#ifndef MIMETYPE_HPP
#define MIMETYPE_HPP

#include <map>
#include <string>

class MimeType {

    private:
    static const std::map<std::string, std::string>& _table();

    MimeType();
    ~MimeType();

    public:
    static std::string getType(const std::string&path);
};

#endif

