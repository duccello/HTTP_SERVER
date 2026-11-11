#include "ConfigTokenizer.hpp"

#include <limits>
#include <iostream>//test
#include <sstream> //std::istringstream 

// ----------------//
//  Public methods //
// ----------------//

// Constructor: open file, check if good, tokenize
ConfigTokenizer::ConfigTokenizer(const std::string& path) {
	_file.open(path.c_str());
	if (!_file)
		throw std::runtime_error("Could not open config file");// TODO catch open file throw on tokenizer
	tokenize();
}

/*
	Tokenize words and recursively checks.
	Split tokens when there is '{' '}' without space at the begining .e.g. `server{{`
	Split tokens when at the end of a word there is a ';'
	Split '{' or '}' at the end without spaces
*/

void ConfigTokenizer::splitToken(const std::string& token, int lineNum) {

	if (!token.empty() && (token[0] == '{' || token[0] == '}')) {

		_tokens.push_back(std::make_pair(token.substr(0,1), lineNum));

		if (token.size() > 1)
			splitToken(token.substr(1), lineNum);
		return;
	}

	if (!token.empty() && token[0] == ';') {

		_tokens.push_back(std::make_pair(";", lineNum));

		if (token.size() > 1)
			splitToken(token.substr(1), lineNum);
		return;
	}

	if (token[token.size()-1] == ';') {
		splitToken(token.substr(0, token.size()-1), lineNum);
		_tokens.push_back(std::make_pair(";", lineNum));
		return;
	}

	if (!token.empty() && (token[token.size() - 1] == '{' 
						|| token[token.size() - 1] == '}')) {
		splitToken(token.substr(0, token.size() - 1), lineNum);
		_tokens.push_back(std::make_pair(token.substr(token.size() - 1), lineNum));
		return;
	}

	size_t semiPos = token.find(';');
	if (semiPos != std::string::npos) {
		splitToken(token.substr(0, semiPos), lineNum);
		splitToken(token.substr(semiPos), lineNum);
		return;
	}

	if (!token.empty()) {
		_tokens.push_back(std::make_pair(token,lineNum));
	}
}

/*
	1. Reads line by line with getline, tracking line number
	2. Strips inline comments (from # to end of line)
	3. Splits each line into tokens by whitespace
	4. Separates ; into its own token
	5. Stores each token as pair<string, lineNumber>
*/
void ConfigTokenizer::tokenize() {

	std::string line;
	int lineNum = 1;

	while (std::getline(_file, line)) {
		size_t comment = line.find('#');
		if (comment != std::string::npos)
			line = line.substr(0, comment);

		std::istringstream iss(line);
		std::string token;

		while (iss >> token) {
			if (token.size() > 1 && (token[0] == '{' || token[0] == '}' || token[0] == ';'
								|| token.find(';') != std::string::npos
								|| token[token.size()-1] == ';' || token[token.size()-1] == '{'
								|| token[token.size()-1] == '}')) {
				splitToken(token, lineNum);
			} else {
				_tokens.push_back(std::make_pair(token, lineNum));
			}
		}
		lineNum++;
	}
}

const std::vector<std::pair<std::string, int> >& ConfigTokenizer::getTokens() const {
	return _tokens;
}