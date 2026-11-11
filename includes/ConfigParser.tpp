#include "utils.hpp"

#include <string>
#include <sstream>
#include <limits>

template <typename T> T ConfigParser::convertToNumber(const std::string& token,
                                    const std::string& directiveName) {
    std::istringstream iss(token);
    double tempValue;

    if (!(iss >> tempValue)) {
        throw std::runtime_error(errorPrefix(_line) + "Invalid number for " + directiveName + ": " + token + RESET);
    }
    
    std::string leftovers; // eg:mb after extract the number in 100mb
    if (iss >> leftovers) {
        throw std::runtime_error(errorPrefix(_line) + "Invalid character format in " + directiveName + ": " + token + RESET);
    }

    if (tempValue > std::numeric_limits<T>::max()) {
        throw std::runtime_error(errorPrefix(_line) + directiveName + " value exceeds maximum limit" + RESET);
    }

    if (tempValue < 0 && std::numeric_limits<T>::min() == 0) {
        throw std::runtime_error(errorPrefix(_line) + directiveName + " cannot be negative" + RESET);
    }

    return static_cast<T>(tempValue);
}