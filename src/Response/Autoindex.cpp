#include "Autoindex.hpp"
#include "Response.hpp"

#include <algorithm> //std::sort
#include <sys/stat.h> // stat()
#include <sstream> //std::ostringstream


// Private methods //

std::string Autoindex::htmlEscape(const std::string& s) {

    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i)
    {
        switch (s[i])
        {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += s[i];
        }
    }
    return out;
}

// readdir fills the entry and check if is directory
std::vector<std::string> Autoindex::listEntries(DIR* dir)
{
	std::vector<std::string> entries;
	struct dirent* ent; //d_name

    // readdir -> check what's there 
	while ((ent = readdir(dir)) != NULL)
	{
        // get the file/dir name
		std::string name = ent->d_name;
		if (name == ".") //skips hidden files
			continue;
        // save the entry
		entries.push_back(name);
	}
	return entries;
}

/*
    Renders one entry everytime
    - stat() gives details from the path: type (file/dir), bytes, permissions.
*/
std::string Autoindex::renderEntry(const std::string& fullPath, const std::string& name){

    struct stat st;

	std::string entryFs = fullPath + "/" + name;

	bool isDir = (stat(entryFs.c_str(), &st) == 0 && S_ISDIR(st.st_mode));

	std::string display = name + (isDir ? "/" : "");
	std::string safeHtml = htmlEscape(display);

	std::ostringstream line;
	line << "<a href=\"" << safeHtml << "\">" << safeHtml << "</a>\n";
	return line.str();
}

// Public methods //

// ls equivalent = opendir+readdir+sort
std::string Autoindex::generateAutoidx(const std::string& fullPath, const std::string& uriPath) {

    // 1. Open directory
    DIR* dir = opendir(fullPath.c_str()); // DIR* handles an open directory
	if (!dir)
		throw HttpException(FORBIDDEN, "Forbidden");

    // 2. List entries
	std::vector<std::string> entries = listEntries(dir);
	closedir(dir);

    // 3. Organize entries
	std::sort(entries.begin(), entries.end());

    // 4. add "/" if that's a directory
	std::string uri = uriPath;
	if (uri.empty() || uri[uri.size() - 1] != '/') //handles memory error if URI is empty
		uri += "/";

    // 5. title
	std::ostringstream html;
	html << "<html>\n<head><title>Index of " << htmlEscape(uri) << "</title></head>\n"
	     << "<body>\n<h1>Index of " << htmlEscape(uri) << "</h1>\n<hr>\n<pre>\n";

    // 6. Render all the entries
	for (size_t i = 0; i < entries.size(); ++i)
		html << renderEntry(fullPath, entries[i]);

	html << "</pre>\n<hr>\n</body>\n</html>\n";
	return html.str();
}