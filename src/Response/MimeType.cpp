#include "MimeType.hpp"

const std::map<std::string, std::string>& MimeType::_table() {

    static std::map<std::string, std::string> mime;

    if (mime.empty()) {
        mime["html"] = "text/html";
        mime["htm"]  = "text/html";
        mime["css"]  = "text/css";
        mime["js"]   = "application/javascript";
        mime["json"] = "application/json";
        mime["png"]  = "image/png";
        mime["jpg"]  = "image/jpeg";
        mime["jpeg"] = "image/jpeg";
        mime["gif"]  = "image/gif";
        mime["svg"]  = "image/svg+xml";
        mime["ico"]  = "image/x-icon";
        mime["txt"]  = "text/plain";
        mime["pdf"]  = "application/pdf";
        mime["mp4"] = "video/mp4";
    }
    return mime;
}

std::string MimeType::getType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
        
    std::string ext = path.substr(dot + 1);
    const std::map<std::string, std::string>& table = _table();
    std::map<std::string, std::string>::const_iterator it = table.find(ext);
    if (it != table.end())
        return it->second;
    return "application/octet-stream";
}
