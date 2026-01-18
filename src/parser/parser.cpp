#include <iostream>

#include "parser.h"
#include "../lexer/tokens.h"

void Parser::parse() {

	std::cout << "======== Starting Parsing  ========" << std::endl;

	// Start parsing by getting the first token
	lexer.getNextToken();

	int16_t currentToken = lexer.getCurrentToken();
	while (currentToken != tok_eof) {
		switch (currentToken) {
			case tok_integer_literal:
				parseIntegerValue();
				std::cout << "Parsed Integer Value" << std::endl;
				break;
			case tok_float_literal:
				parseFloatValue();
				std::cout << "Parsed Float Value" << std::endl;
				break;
			case tok_true:
			case tok_false:
				parseBoolValue();
				std::cout << "Parsed Bool Value" << std::endl;
				break;
			case tok_char_literal:
				parseCharValue();
				std::cout << "Parsed Char Value" << std::endl;
				break;
			case tok_identifier:
				parseIdentifier();
				std::cout << "Parsed Identifier" << std::endl;
				break;
			case tok_class:
				// For now, we can assum this class to inherit from Application
				parseClassDefinition();
				break;
			default:
				// Ignore other tokens for now
				std::cerr << "Ignoring token: " << static_cast<Token>(currentToken) << std::endl;
				break;
		}

		currentToken = lexer.getNextToken();
	}

	std::cout << "======== Parsing completed ========." << std::endl;
}
