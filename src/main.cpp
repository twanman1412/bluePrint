#include <iostream>
#include <string>
#include <fstream>

#include "lexer/lexer.h"
#include "parser/parser.h"

int main (int argc, char *argv[]) {
	
	if (argc <= 1) {
		std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
		return 0;
	}

	// Read the file from the command line argument
	std::string filename = argv[1];
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return 1;
	}

	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	Lexer lexer = Lexer(text);
	std::cout << "Tokens:" << std::endl;
	while (true) {
		int16_t token = lexer.getNextToken();
		if (token == tok_eof) {
			break;
		}
		std::cout << "\t" << static_cast<Token>(token) << std::endl;
	}

	lexer = Lexer(text); // Reinitialize lexer for parsing
	Parser parser = Parser(lexer);
	parser.parse();

	return 0;
}
