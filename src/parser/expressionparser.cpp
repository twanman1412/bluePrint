#include <iostream>
#include <map>

#include "parser.hpp"
#include "../lexer/tokens.hpp"

std::unique_ptr<ExprAST> Parser::parseExpression() {
	std::cout << "Parsing Expression..." << std::endl;
	int16_t currentToken = lexer.getCurrentToken();
	switch (currentToken) {
		case '(':
			return parseParenExpression();
		default:
			std::cerr << "Error: Unknown expression starting token." << std::endl;
			return nullptr;
	}
}

std::unique_ptr<ExprAST> Parser::parseParenExpression() {
	std::cout << "Parsing Parenthesized Expression..." << std::endl;

	// Expect '('
	if (lexer.getCurrentToken() != '(') {
		std::cerr << "Error: Expected '(' at the start of parenthesized expression." << std::endl;
		return nullptr;
	}

	// Parse the inner expression
	lexer.getNextToken(); // consume '('
	std::unique_ptr<ExprAST> expr = parseBinaryExpression();
	if (!expr) {
		return nullptr;
	}

	// Expect ')'
	if (lexer.getCurrentToken() != ')') {
		std::cerr << "Error: Expected ')' at the end of parenthesized expression." << std::endl;
		return nullptr;
	}
	lexer.getNextToken(); // consume ')'

	return expr;
}

static std::map<std::string, int> getOperatorPrecedenceMap() {
	return {
		{"&&", 5},
		{"||", 5},
		{"==", 10},
		{"!=", 10},
		{"<", 15},
		{">", 15},
		{"<=", 15},
		{">=", 15},
		{"+", 20},
		{"-", 20},
		{"*", 40},
		{"/", 40},
	};
}

static int getTokenPrecedence(std::string op) {
	static std::map<std::string, int> operatorPrecedence = getOperatorPrecedenceMap();
	auto it = operatorPrecedence.find(op);
	if (it != operatorPrecedence.end()) {
		return it->second;
	}
	return -1;
}

std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int exprPrecedence, std::unique_ptr<ExprAST> lhs) {
	while (true) {

		int16_t currentToken = lexer.getCurrentToken();
		std::string op;
		switch (currentToken) {
			case '+': op = "+"; break;
			case '-': op = "-"; break;
			case '*': op = "*"; break;
			case '/': op = "/"; break;
			case '=': {
						  currentToken = lexer.getNextToken();
						  if (currentToken != '=') {
							  return lhs;
						  }
						  op = "=="; 
						  break;
					  }
			case '!': {
						  currentToken = lexer.getNextToken();
						  if (currentToken != '=') {
							  return lhs;
						  }
						  op = "!="; 
						  break;
					  }
			case '<': {
						  currentToken = lexer.getNextToken();
						  if (currentToken == '=') {
							  op = "<="; 
						  } else {
							  op = "<";
							  // Put back the tokens
							  lexer.ungetCurrentToken();
						  }
						  break;
					  }
			case '>': {
						  currentToken = lexer.getNextToken();
						  if (currentToken == '=') {
							  op = ">="; 
						  } else {
							  op = ">"; 
							  // Put back the tokens
							  lexer.ungetCurrentToken();
						  }
						  break;
					  }
			case '&': {
						  currentToken = lexer.getNextToken();
						  if (currentToken != '&') {
							  return lhs;
						  }
						  op = "&&"; 
						  break;
					  }
			case '|': {
						  currentToken = lexer.getNextToken();
						  if (currentToken != '|') {
							  return lhs;
						  }
						  op = "||"; 
						  break;
					  }
			default:
				return lhs;
		}

		int tokenPrecedence = getTokenPrecedence(op);
		if (tokenPrecedence < exprPrecedence) {
			return lhs;
		}

		lexer.getNextToken(); // consume operator
		std::unique_ptr<ExprAST> rhs = parsePrimaryExpression();

		if (!rhs) {
			return nullptr;
		}

	}
}
