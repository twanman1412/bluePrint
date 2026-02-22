#include <iostream>
#include <map>

#include "parser.hpp"
#include "../lexer/tokens.hpp"

static std::map<int16_t, int> getOperatorPrecedenceMap() {
    return {
        {'|', 5},
        {'&', 6},
        {tok_equal_equal, 10},
        {tok_not_equal, 10},
        {'<', 15},
        {'>', 15},
        {tok_less_equal, 15},
        {tok_greater_equal, 15},
        {'+', 20},
        {'-', 20},
        {'*', 40},
        {'/', 40},
        {'%', 40},
    };
}

static int getTokenPrecedence(int16_t token) {
    static std::map<int16_t, int> operatorPrecedence = getOperatorPrecedenceMap();
    auto it = operatorPrecedence.find(token);
    if (it != operatorPrecedence.end()) {
        return it->second;
    }
    return -1;
}

static int getBinaryOperatorToken(int16_t token) {
    switch (token) {
        case '+':
            return BinaryExprAST::PLUS;
        case '-':
            return BinaryExprAST::MINUS;
        case '*':
            return BinaryExprAST::MULTIPLY;
        case '/':
            return BinaryExprAST::DIVIDE;
        case '%':
            return BinaryExprAST::MODULO;
        case tok_equal_equal:
            return BinaryExprAST::EQUAL;
        case tok_not_equal:
            return BinaryExprAST::NOT_EQUAL;
        case '<':
            return BinaryExprAST::LESS_THAN;
        case tok_less_equal:
            return BinaryExprAST::LESS_EQUAL;
        case '>':
            return BinaryExprAST::GREATER_THAN;
        case tok_greater_equal:
            return BinaryExprAST::GREATER_EQUAL;
        case '&':
            return BinaryExprAST::LOGICAL_AND;
        case '|':
            return BinaryExprAST::LOGICAL_OR;
        default:
            return -1;
    }
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    Parser::logln("Parsing Expression...");
    auto lhs = parseUnaryExpression();
    if (!lhs) {
        return nullptr;
    }
    return parseBinaryOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::parseBinaryExpression() {
    return parseExpression();
}

std::unique_ptr<ExprAST> Parser::parseUnaryExpression() {
    const int16_t currentToken = lexer.getCurrentToken();
    if (currentToken == '-' || currentToken == '!') {
        const int unaryOperator = (currentToken == '-') ? UnaryExprAST::NEGATE : UnaryExprAST::LOGICAL_NOT;
        lexer.getNextToken();
        auto operand = parseUnaryExpression();
        if (!operand) {
            return nullptr;
        }
        return std::make_unique<UnaryExprAST>(unaryOperator, std::move(operand));
    }

    return parsePrimaryExpression();
}

std::unique_ptr<ExprAST> Parser::parseParenExpression() {
    Parser::logln("Parsing Parenthesized Expression...");

    if (lexer.getCurrentToken() != '(') {
        std::cerr << "Error: Expected '(' at the start of parenthesized expression." << std::endl;
        return nullptr;
    }

    lexer.getNextToken();
    std::unique_ptr<ExprAST> expr = parseExpression();
    if (!expr) {
        return nullptr;
    }

    if (lexer.getCurrentToken() != ')') {
        std::cerr << "Error: Expected ')' at the end of parenthesized expression." << std::endl;
        return nullptr;
    }

    lexer.getNextToken();
    return expr;
}

std::unique_ptr<ExprAST> Parser::parseBinaryOpRHS(int exprPrecedence, std::unique_ptr<ExprAST> lhs) {
    while (true) {
        const int16_t currentToken = lexer.getCurrentToken();
        const int tokenPrecedence = getTokenPrecedence(currentToken);

        if (tokenPrecedence < exprPrecedence) {
            return lhs;
        }

        const int binaryOperator = getBinaryOperatorToken(currentToken);
        if (binaryOperator < 0) {
            return lhs;
        }

        lexer.getNextToken();
        auto rhs = parseUnaryExpression();
        if (!rhs) {
            return nullptr;
        }

        const int nextPrecedence = getTokenPrecedence(lexer.getCurrentToken());
        if (tokenPrecedence < nextPrecedence) {
            rhs = parseBinaryOpRHS(tokenPrecedence + 1, std::move(rhs));
            if (!rhs) {
                return nullptr;
            }
        }

        lhs = std::make_unique<BinaryExprAST>(binaryOperator, std::move(lhs), std::move(rhs));
    }
}
