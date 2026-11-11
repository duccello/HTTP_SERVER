#include "utils.hpp"

#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <iostream>

std::string readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		return "";
	}
	std::ostringstream ss;

	ss << file.rdbuf();
	return ss.str();
}

std::string intToString(int nb)
{
    std::stringstream ss;

    ss << nb;
    return (ss.str());
}

std::string errorPrefix(int _line) {
    std::ostringstream oss;
    oss << FATAL_ERROR << "[line " << _line << "] ";
    return oss.str();
}

int writeLoop(int fd, const std::string& data)
{
    size_t total = 0;

    while (total < data.size())
    {
        ssize_t n = write(fd, data.data() + total, data.size() - total);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;
			return (1);
        }

        total += static_cast<size_t>(n);
    }
	return (0);
}

uint8_t	get_return_val(int status)
{
	uint8_t	ret_val;

	if ((status & 127) == 0)
		ret_val = (status >> 8);
	else
		ret_val = 128 + (status & 127);
	return (ret_val);
}