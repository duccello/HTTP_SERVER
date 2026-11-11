#ifndef CONFIGTOKENIZER_HPP
#define CONFIGTOKENIZER_HPP

#include <fstream>
#include <vector>

// Helper class to convert the configfile into tokens
/*
	Main tokens: ; { }
	MANDATORY: server listen

	OPTIONALS - with defaults:
*/

/*
	- read file chay by char
	- split words and symbols
	- ignor tabs and spaces
	- ignor comments
	- detect {};
	- Create the token list
*/
class ConfigTokenizer {

	private:

		// Attributes
		std::ifstream _file;

		std::vector<std::pair<std::string, int> > _tokens;

		// OFC -> std::ifstream can't be copied

		ConfigTokenizer();

		ConfigTokenizer (const ConfigTokenizer& other);

		ConfigTokenizer& operator=(const ConfigTokenizer& other);

		void splitToken(const std::string& token, int LineNum);

	public:

		ConfigTokenizer(const std::string& path); // config file path

		void tokenize(); // inside there is the local variable std::string token

		// Getters
		const std::vector<std::pair<std::string, int> >& getTokens() const;// all tokens ready

};

#endif