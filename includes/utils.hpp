#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <stdint.h>

#define FATAL_ERROR     "\033[31m"
#define WARNING         "\033[33m"
#define RESET           "\033[0m"

std::string readFile(const std::string &path);

std::string intToString(int nb);

std::string errorPrefix(int line);

int writeLoop(int fd, const std::string& data);

uint8_t	get_return_val(int status);

#endif