#include "parser.hpp"
#include "../lexer/tokens.hpp"

void Parser::parse() {

	Parser::logln("======== Starting Lexing  ========");

	// Start parsing by getting the first token
	lexer.getNextToken();

	int16_t currentToken = lexer.getCurrentToken();
	while (currentToken != tok_eof) {
		switch (currentToken) {
			case tok_integer_literal:
				parseIntegerValue();
				Parser::logln("Parsed Integer Value");
				break;
			case tok_float_literal:
				parseFloatValue();
				Parser::logln("Parsed Float Value");
				break;
			case tok_true:
			case tok_false:
				parseBoolValue();
				Parser::logln("Parsed Bool Value");
				break;
			case tok_char_literal:
				parseCharValue();
				Parser::logln("Parsed Char Value");
				break;
			case tok_identifier:
				parseIdentifier();
				Parser::logln("Parsed Identifier");
				break;
			case tok_class:
				// For now, we can assum this class to inherit from Application
				parseClassDefinition();
				break;
			default:
				perror("Unknown token encountered during parsing.");
				exit(-1);
		}

		currentToken = lexer.getNextToken();
	}

	Parser::logln("======== Parsing completed ========.");
}
