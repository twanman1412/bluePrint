#include <iostream>

#include "parser.hpp"
#include "../lexer/tokens.hpp"

std::unique_ptr<StmtAST> Parser::parseStatement() {
    Parser::logln("Parsing Statement...");

    int16_t currentToken = lexer.getCurrentToken();

    if (currentToken == '{') {
        lexer.getNextToken();
        std::vector<std::unique_ptr<StmtAST>> statements;
        while (lexer.getCurrentToken() != '}') {
            auto nestedStatement = parseStatement();
            if (!nestedStatement) {
                return nullptr;
            }
            statements.push_back(std::move(nestedStatement));
        }

        lexer.getNextToken();
        return std::make_unique<BlockStmtAST>(std::move(statements));
    }

    if (currentToken == tok_if) {
        if (lexer.getNextToken() != '(') {
            std::cerr << "Error: Expected '(' after if." << std::endl;
            return nullptr;
        }

        lexer.getNextToken();
        auto condition = parseExpression();
        if (!condition) {
            return nullptr;
        }

        if (lexer.getCurrentToken() != ')') {
            std::cerr << "Error: Expected ')' after if condition." << std::endl;
            return nullptr;
        }

        lexer.getNextToken();
        auto thenStatement = parseStatement();
        if (!thenStatement) {
            return nullptr;
        }

        std::unique_ptr<StmtAST> elseStatement = nullptr;
        if (lexer.getCurrentToken() == tok_else) {
            lexer.getNextToken();
            elseStatement = parseStatement();
            if (!elseStatement) {
                return nullptr;
            }
        }

        return std::make_unique<IfStmtAST>(std::move(condition), std::move(thenStatement), std::move(elseStatement));
    }

    if (currentToken == tok_while) {
        if (lexer.getNextToken() != '(') {
            std::cerr << "Error: Expected '(' after while." << std::endl;
            return nullptr;
        }

        lexer.getNextToken();
        auto condition = parseExpression();
        if (!condition) {
            return nullptr;
        }

        if (lexer.getCurrentToken() != ')') {
            std::cerr << "Error: Expected ')' after while condition." << std::endl;
            return nullptr;
        }

        lexer.getNextToken();
        auto body = parseStatement();
        if (!body) {
            return nullptr;
        }

        return std::make_unique<WhileStmtAST>(std::move(condition), std::move(body));
    }

    if (TokenUtils::isPrimitiveTypeToken(currentToken)) {
        std::unique_ptr<TypeAST> elementType = ParserUtils::getPrimitiveTypeFromToken(currentToken);
        std::unique_ptr<TypeAST> variableType;

        currentToken = lexer.getNextToken();

        // Check for array type suffix: T[]
        if (currentToken == '[') {
            currentToken = lexer.getNextToken();
            if (currentToken != ']') {
                std::cerr << "Error: Expected ']' to close array type." << std::endl;
                return nullptr;
            }
            variableType = std::make_unique<ArrayTypeAST>(std::move(elementType));
            currentToken = lexer.getNextToken();
        } else {
            variableType = std::move(elementType);
        }

        if (currentToken != tok_identifier) {
            std::cerr << "Error: Expected variable name identifier." << std::endl;
            return nullptr;
        }

        const std::string variableName = lexer.getIdentifierName();

        currentToken = lexer.getNextToken();
        if (currentToken != '=') {
            std::cerr << "Error: Expected '=' after variable name." << std::endl;
            return nullptr;
        }

        lexer.getNextToken();
        auto initializer = parseExpression();
        if (!initializer) {
            return nullptr;
        }

        if (lexer.getCurrentToken() != ';') {
            std::cerr << "Error: Expected ';' after variable declaration." << std::endl;
            return nullptr;
        }

        lexer.getNextToken();
        return std::make_unique<VarDeclStmtAST>(std::move(variableType), variableName, std::move(initializer));
    }

    if (currentToken == tok_identifier) {
        const std::string identifierName = lexer.getIdentifierName();
        currentToken = lexer.getNextToken();

        if (currentToken == '[') {
            lexer.getNextToken();
            auto indexExpr = parseExpression();
            if (!indexExpr) return nullptr;
            if (lexer.getCurrentToken() != ']') {
                std::cerr << "Error: Expected ']' after array index in assignment." << std::endl;
                return nullptr;
            }
            if (lexer.getNextToken() != '=') {
                std::cerr << "Error: Expected '=' after array index in assignment." << std::endl;
                return nullptr;
            }
            lexer.getNextToken();
            auto valueExpr = parseExpression();
            if (!valueExpr) return nullptr;
            if (lexer.getCurrentToken() != ';') {
                std::cerr << "Error: Expected ';' after array index assignment." << std::endl;
                return nullptr;
            }
            lexer.getNextToken();
            return std::make_unique<IndexAssignStmtAST>(identifierName, std::move(indexExpr), std::move(valueExpr));
        }

        if (identifierName == "Defaultlogger" && currentToken == '.') {
            currentToken = lexer.getNextToken();
            if (currentToken != tok_identifier || lexer.getIdentifierName() != "logln") {
                std::cerr << "Error: Expected Defaultlogger.logln call." << std::endl;
                return nullptr;
            }

            currentToken = lexer.getNextToken();
            if (currentToken != '(') {
                std::cerr << "Error: Expected '(' in Defaultlogger.logln call." << std::endl;
                return nullptr;
            }

            lexer.getNextToken();
            auto loggedValue = parseExpression();
            if (!loggedValue) {
                return nullptr;
            }

            if (lexer.getCurrentToken() != ')') {
                std::cerr << "Error: Expected ')' in Defaultlogger.logln call." << std::endl;
                return nullptr;
            }

            if (lexer.getNextToken() != ';') {
                std::cerr << "Error: Expected ';' after Defaultlogger.logln call." << std::endl;
                return nullptr;
            }

            lexer.getNextToken();
            return std::make_unique<PrintStmtAST>(std::move(loggedValue));
        }

        if (currentToken == '=') {
            lexer.getNextToken();
            auto assignedValue = parseExpression();
            if (!assignedValue) {
                return nullptr;
            }

            if (lexer.getCurrentToken() != ';') {
                std::cerr << "Error: Expected ';' after assignment." << std::endl;
                return nullptr;
            }

            lexer.getNextToken();
            return std::make_unique<AssignmentStmtAST>(identifierName, std::move(assignedValue));
        }

        std::cerr << "Error: Unsupported identifier statement." << std::endl;
        return nullptr;
    }

    std::cerr << "Error: Unknown statement starting token." << std::endl;
    return nullptr;
}
