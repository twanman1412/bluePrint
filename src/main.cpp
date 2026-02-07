#include <iostream>
#include <string>
#include <fstream>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main (int argc, char *argv[]) 
{
	if (argc <= 1) 
	{
		std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
		return 0;
	}

	std::string arg1 = argv[1];
	if (arg1 == "--help" || arg1 == "-h") 
	{
		std::cout << "Usage: " << argv[0] << " [options] <source_file>" << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "  --help, -h       Show this help message and exit" << std::endl;
		std::cout << "  --verbose, -v    Enable verbose output during lexing and parsing" << std::endl;
		return 0;
	}

	bool verbose = false;
	if (arg1 == "--verbose" || arg1 == "-v") {
		verbose = true;
		if (argc <= 2) {
			std::cerr << "Error: Missing source file argument after verbose flag." << std::endl;
			return 1;
		}
	}

	// Read the file from the command line argument
	std::string filename;
	if (verbose) 
		filename = argv[2];
	else 
		filename = argv[1];
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return 1;
	}

	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	Lexer lexer = Lexer(text);
	if (verbose)
	{
		std::cout << "Tokens:" << std::endl;
		while (true) {
			int16_t token = lexer.getNextToken();
			if (token == tok_eof) {
				break;
			}
			std::cout << "\t" << static_cast<Token>(token) << std::endl;
		}

		lexer = Lexer(text); // Reinitialize lexer for parsing
	}

	Parser parser = Parser(lexer, verbose);
	parser.parse();

	return 0;
}
