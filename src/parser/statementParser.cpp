#include <iostream>

#include "parser.hpp"
#include "../lexer/tokens.hpp"

std::unique_ptr<StmtAST> Parser::parseStatement() {

	Parser::logln("Parsing Statement...");

	int16_t currentToken = lexer.getCurrentToken();
	switch (currentToken) {
		case tok_i32:
		case tok_f32:
		case tok_bool:
		case tok_char: {
			   // Variable Declaration
			   std::unique_ptr<TypeAST> varType = ParserUtils::getPrimitiveTypeFromToken(currentToken);

			   // Get variable name
			   currentToken = lexer.getNextToken();
			   if (currentToken != tok_identifier) {
				   std::cerr << "Error: Expected variable name identifier." << std::endl;
				   return nullptr;
			   }

			   std::string varName = lexer.getIdentifierName();

			   // Expect '='
			   currentToken = lexer.getNextToken();
			   if (currentToken != '=') {
				   std::cerr << "Error: Expected '=' after variable name." << std::endl;
				   return nullptr;
			   }

			   // Parse initializer expression
			   while (currentToken != ';')
			   	currentToken = lexer.getNextToken();

			   return std::make_unique<VarDeclStmtAST>(
					   std::move(varType), 
					   varName, 
					   std::make_unique<IntegerExprAST>(0)
				   );
			}
		default:
			std::cerr << "Error: Unknown statement starting token." << std::endl;
			return nullptr;
	}
}

