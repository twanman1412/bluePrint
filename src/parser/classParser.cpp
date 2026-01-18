#include <iostream>
#include <array>
#include <algorithm>
#include <memory>

#include "parser.h"
#include "../lexer/tokens.h"

std::unique_ptr<ClassAST> Parser::parseClassDefinition() {
	std::cout << "Parsing Class Definition..." << std::endl;

	// Expect 'class' keyword
	if (lexer.getCurrentToken() != tok_class) {
		std::cerr << "Error: Expected 'class' keyword." << std::endl;
		return nullptr;
	}

	// Get class name
	int16_t currentToken = lexer.getNextToken();
	if (currentToken != tok_identifier) {
		std::cerr << "Error: Expected class name identifier." << std::endl;
		return nullptr;
	}
	std::string className = lexer.getIdentifierName();

	// Expect ':'
	currentToken = lexer.getNextToken();
	if (currentToken != ':') {
		std::cerr << "Error: Expected ':' after class name." << std::endl;
		return nullptr;
	}

	// Expect 'Application' as base class for now
	currentToken = lexer.getNextToken();
	if (currentToken != tok_identifier && lexer.getIdentifierName() != "Application") {
		std::cerr << "Error: Expected 'Application' class name after ':'." << std::endl;
		return nullptr;
	}

	const std::vector<std::string> blueprintNames = {"Application"};

	// Expect '{'
	currentToken = lexer.getNextToken();
	if (currentToken != '{') {
		std::cerr << "Error: Expected '{' after class name." << std::endl;
		return nullptr;
	}

	// Move to the next token after '{'
	lexer.getNextToken(); 

	std::vector<std::unique_ptr<MethodImplAST>> methodImpls;
	while (true) {
		currentToken = lexer.getCurrentToken();
		if (currentToken == '}') {
			break; // End of class body
		}
		 std::cout << "Current Token at start of method impl: " << static_cast<Token>(currentToken) << std::endl;

		auto implementation = this->parseMethodImplementation();
		if (!implementation || implementation == nullptr) {
			std::cout << "Failed to parse method implementation." << std::endl;
			return nullptr;
		}
		std::cout << "Parsed Method Implementation" << std::endl;
		std::cout << "\t Name: " << implementation->getName() << std::endl;
		std::cout << "\t Params Count: " << implementation->getParams().size() << std::endl;
		methodImpls.push_back(std::move(implementation));
	}

	// Consume the closing '}'
	lexer.getNextToken();

	return std::make_unique<ClassAST>(className, std::move(methodImpls), std::move(blueprintNames));
}

std::unique_ptr<MethodImplAST> Parser::parseMethodImplementation() {
	std::cout << "Parsing Method Implementation..." << std::endl;

	int16_t currentToken = lexer.getCurrentToken();
	const auto accessModifiers = TokenUtils::getAccessModifierTokens();
	if (std::find(accessModifiers.begin(), accessModifiers.end(), currentToken) == accessModifiers.end()) {
		std::cerr << "Error: Expected access modifier before method implementation." << std::endl;
		return nullptr;
	}

	auto accessModifier = ParserUtils::getAccessModifierFromToken(currentToken);

	std::unique_ptr<TypeAST> returnType;
	currentToken = lexer.getNextToken();

	switch (currentToken) {
		case tok_void:
			returnType = std::make_unique<PrimitiveTypeAST>(PrimitiveTypeAST::VOID);
			break;
		default:
			std::cerr << "Error: Non-void return type." << std::endl;
			return nullptr;
	}

	// Get method name
	currentToken = lexer.getNextToken();
	if (currentToken != tok_identifier) {
		std::cerr << "Error: Expected method name identifier." << std::endl;
		return nullptr;
	}
	std::string methodName = lexer.getIdentifierName();

	// Expect '('
	currentToken = lexer.getNextToken();
	if (currentToken != '(') {
		std::cerr << "Error: Expected '(' after method name." << std::endl;
		return nullptr;
	}

	currentToken = lexer.getNextToken(); // Move to first parameter or ')'
	std::vector<std::unique_ptr<TypedIdentifierAST>> params;
	while (currentToken != ')') {
		if (!TokenUtils::isPrimitiveTypeToken(currentToken)) {
			std::cerr << "Error: Expected parameter type." << std::endl;
			return nullptr;
		}
		std::unique_ptr<TypeAST> paramType = ParserUtils::getPrimitiveTypeFromToken(currentToken);

		currentToken = lexer.getNextToken();
		std::cout << "Current Token at middle of method impl: " << static_cast<Token>(currentToken) << std::endl;
		if (currentToken != tok_identifier) {
			std::cerr << "Error: Expected parameter." << std::endl;
			return nullptr;
		}
		const std::string paramName = lexer.getIdentifierName();

		params.push_back(ParserUtils::makeTypedIdentifier(std::move(paramType), paramName));

		currentToken = lexer.getNextToken();
		if (currentToken == ')') {
			break; // End of parameters
		}

		if (currentToken != ',') {
			std::cerr << "Error: Expected ',' or ')' after parameter." << std::endl;
			return nullptr;
		}

		currentToken = lexer.getNextToken(); // Move to next parameter type
	}

	// Expect '{'
	currentToken = lexer.getNextToken();
	if (currentToken != '{') {
		std::cerr << "Error: Expected '{' to start method body." << std::endl;
		return nullptr;
	}

	std::vector<std::unique_ptr<StmtAST>> body;
	while (true) {
		currentToken = lexer.getNextToken();
		if (currentToken == '}') {
			break; // End of method body
		}

		std::unique_ptr<StmtAST> stmt = parseStatement();
		if (!stmt) {
			return nullptr;
		}
		body.push_back(std::move(stmt));
	}

	// Expect '}'
	if (currentToken != '}') {
		std::cerr << "Error: Expected '}' to end method body." << std::endl;
		return nullptr;
	}
	lexer.getNextToken(); // Move to next token after method implementation

	return std::make_unique<MethodImplAST>(
		std::vector<std::unique_ptr<AccessModifierAST>>{}, 
		std::move(returnType),
		methodName,
		std::move(params),
		std::move(body)
	);
}
