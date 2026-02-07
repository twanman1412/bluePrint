#include <iostream>
#include <memory>

#include "parser.hpp"
#include "../lexer/tokens.hpp"

std::unique_ptr<ExprAST> Parser::parsePrimaryExpression() {
	int16_t currentToken = lexer.getCurrentToken();
	switch (currentToken) {
		case tok_integer_literal:
			return parseIntegerValue();
		case tok_float_literal:
			return parseFloatValue();
		case tok_true:
		case tok_false:
			return parseBoolValue();
		case tok_char_literal:
			return parseCharValue();
		case tok_identifier:
			return parseIdentifier();
		default:
			std::cerr << "Error: Unknown primary expression token." << std::endl;
			return nullptr;
	}
}

std::unique_ptr<IntegerExprAST> Parser::parseIntegerValue() {
    int value = lexer.getIntegerValue();
    lexer.getNextToken();
    return std::make_unique<IntegerExprAST>(value);
}

std::unique_ptr<FloatExprAST> Parser::parseFloatValue() {
    double value = lexer.getFloatValue();
    lexer.getNextToken();
    return std::make_unique<FloatExprAST>(value);
}

std::unique_ptr<BoolExprAST> Parser::parseBoolValue() {
    bool value = lexer.getBoolValue();
    lexer.getNextToken();
    return std::make_unique<BoolExprAST>(value);
}

std::unique_ptr<CharExprAST> Parser::parseCharValue() {
    char value = lexer.getCharValue();
    lexer.getNextToken();
    return std::make_unique<CharExprAST>(value);
}

std::unique_ptr<IdentifierExprAST> Parser::parseIdentifier() {
    std::string name = lexer.getIdentifierName();
    lexer.getNextToken();
    return std::make_unique<IdentifierExprAST>(name);
}
