#include <iostream>
#include <memory>
#include <cstdint>

#include "parser.hpp"
#include "../lexer/tokens.hpp"

std::unique_ptr<ExprAST> Parser::parsePrimaryExpression() {
	int16_t currentToken = lexer.getCurrentToken();
	switch (currentToken) {
		case '(':
			return parseParenExpression();
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
			{
				std::string name = lexer.getIdentifierName();
				lexer.getNextToken();
				if (lexer.getCurrentToken() == '[') {
					lexer.getNextToken();
					auto indexExpr = parseExpression();
					if (!indexExpr) return nullptr;
					if (lexer.getCurrentToken() != ']') {
						std::cerr << "Error: Expected ']' after array index." << std::endl;
						return nullptr;
					}
					lexer.getNextToken();
					return std::make_unique<IndexExprAST>(name, std::move(indexExpr));
				}
				return std::make_unique<IdentifierExprAST>(name);
			}
		case tok_str_literal:
			return parseStrValue();
		case tok_new:
			return parseArrayNew();
		case '{':
			return parseArrayLiteral();
		default:
			std::cerr << "Error: Unknown primary expression token." << std::endl;
			std::cerr << "Token: " << currentToken << std::endl;
			return nullptr;
	}
}

std::unique_ptr<IntegerExprAST> Parser::parseIntegerValue() {
	int64_t value = lexer.getIntegerValue();
    lexer.getNextToken();
    return std::make_unique<IntegerExprAST>(value);
}

std::unique_ptr<FloatExprAST> Parser::parseFloatValue() {
    double value = lexer.getFloatValue();
    lexer.getNextToken();
    return std::make_unique<FloatExprAST>(value);
}

std::unique_ptr<BoolExprAST> Parser::parseBoolValue() {
	bool value = lexer.getCurrentToken() == tok_true;
    lexer.getNextToken();
    return std::make_unique<BoolExprAST>(value);
}

std::unique_ptr<CharExprAST> Parser::parseCharValue() {
    char value = lexer.getCharValue();
    lexer.getNextToken();
    return std::make_unique<CharExprAST>(value);
}

std::unique_ptr<StrExprAST> Parser::parseStrValue() {
    std::string value = lexer.getStringValue();
    lexer.getNextToken();
    return std::make_unique<StrExprAST>(value);
}

std::unique_ptr<IdentifierExprAST> Parser::parseIdentifier() {
    std::string name = lexer.getIdentifierName();
    lexer.getNextToken();
    return std::make_unique<IdentifierExprAST>(name);
}

std::unique_ptr<ArrayLiteralExprAST> Parser::parseArrayLiteral() {
	lexer.getNextToken(); // consume '{'
	std::vector<std::unique_ptr<ExprAST>> elements;
	while (lexer.getCurrentToken() != '}') {
		auto elem = parseExpression();
		if (!elem) return nullptr;
		elements.push_back(std::move(elem));
		if (lexer.getCurrentToken() == ',') {
			lexer.getNextToken();
		} else if (lexer.getCurrentToken() != '}') {
			std::cerr << "Error: Expected ',' or '}' in array literal." << std::endl;
			return nullptr;
		}
	}
	lexer.getNextToken(); // consume '}'
	return std::make_unique<ArrayLiteralExprAST>(std::move(elements));
}

std::unique_ptr<ArrayNewExprAST> Parser::parseArrayNew() {
	// current token is tok_new
	int16_t typeToken = lexer.getNextToken();
	auto elementType = ParserUtils::getPrimitiveTypeFromToken(typeToken);
	if (!elementType) {
		std::cerr << "Error: Expected element type after 'new'." << std::endl;
		return nullptr;
	}
	if (lexer.getNextToken() != '[') {
		std::cerr << "Error: Expected '[' after element type in new expression." << std::endl;
		return nullptr;
	}
	lexer.getNextToken(); // move past '['
	auto sizeExpr = parseExpression();
	if (!sizeExpr) return nullptr;
	if (lexer.getCurrentToken() != ']') {
		std::cerr << "Error: Expected ']' after array size." << std::endl;
		return nullptr;
	}
	lexer.getNextToken(); // consume ']'
	return std::make_unique<ArrayNewExprAST>(std::move(elementType), std::move(sizeExpr));
}
