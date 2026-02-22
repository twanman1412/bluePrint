#include "parser.hpp"
#include "../lexer/tokens.hpp"
#include "../codegen/CodeGenerator.hpp"

bool Parser::parse(CodeGenerator& generator) {

	Parser::logln("======== Starting Lexing  ========");

	// Start parsing by getting the first token
	lexer.getNextToken();
	bool hasGeneratedIR = false;

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
				{
					auto classAST = parseClassDefinition();
					if (!classAST) {
						std::cerr << "Error: Failed to parse class definition." << std::endl;
						return false;
					}

					if (!classAST->codegen(generator)) {
						std::cerr << "Error: Failed to generate LLVM IR for class." << std::endl;
						return false;
					}

					hasGeneratedIR = true;
				}
				break;
			default:
				perror("Unknown token encountered during parsing.");
				return false;
		}

		currentToken = lexer.getNextToken();
	}

	if (hasGeneratedIR && verbose) {
		generator.dumpIR();
	}

	Parser::logln("======== Parsing completed ========.");
	if (!hasGeneratedIR) {
		std::cerr << "Error: No compilable class/program was found in source." << std::endl;
		return false;
	}

	return true;
}
