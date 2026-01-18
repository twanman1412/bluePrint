#include <iostream>
#include <unordered_map>
#include <cctype>

#include "lexer.h"
#include "tokens.h"

// Initialize the keyword lookup table
std::unordered_map<std::string, Token> Lexer::keywords = {
    // Primitive types
    {"i32", tok_i32},
    {"f32", tok_f32},
    {"bool", tok_bool}, 
	{"char", tok_char}, 
	{"void", tok_void},
    
    // Literals
    {"true", tok_true}, 
	{"false", tok_false},
    
    // Declaration keywords
    {"class", tok_class},
    
    // Control flow
    {"if", tok_if}, 
	{"else", tok_else}, 
	{"while", tok_while}, 

	// Access modifiers
	{"public", tok_public},
	};

Lexer::Lexer(std::string sourceCode) : source(sourceCode) {
	this->currentToken = 0;
}

char Lexer::getchar() {
	if (currentIndex >= source.size()) {
		return EOF;
	}
	return source[currentIndex++];
}

int16_t Lexer::getNextToken() {
    int lastChar = ' ';

    while (isspace(lastChar))
        lastChar = this->getchar();

    if (lastChar == EOF) {
        return tok_eof;
	}

    if (isalpha(lastChar)) {
        std::string identifierStr;
        do {
            identifierStr += lastChar;
            lastChar = this->getchar();
        } while (isalnum(lastChar));
		ungetCurrentToken(); // Put back the last read character

        this->currentToken = getKeywordToken(identifierStr);
		return this->currentToken;
    }

    if (isdigit(lastChar) || lastChar == '.') {
        std::string numStr;
        bool isFloat = false;
        do {
            if (lastChar == '.' && !isFloat) {
                isFloat = true;
            } else if (lastChar == '.' && isFloat) {
				std::cerr << "Error: Invalid number format with multiple decimal points." << std::endl;
				return tok_eof;
            }

            numStr += lastChar;
            lastChar = this->getchar();
        } while (isdigit(lastChar) || lastChar == '.');

        // Handle number conversion and return appropriate token
        if (isFloat) {
            this->floatValue = strtod(numStr.c_str(), nullptr);
            this->currentToken = tok_float_literal;
            return this->currentToken;
        } else {
            this->integerValue = strtoll(numStr.c_str(), nullptr, 10);
            this->currentToken = tok_integer_literal;
            return this->currentToken;
        }
    }

    if (lastChar == '/') {
        lastChar = this->getchar();
        if (lastChar == '/') {
            // Single line comment
            do {
                lastChar = this->getchar();
            } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

            if (lastChar != EOF)
                return getNextToken();
        } else if (lastChar == '*') {
            // Multi-line comment
            while (true) {
                lastChar = this->getchar();
                if (lastChar == EOF)
                    return tok_eof;
                if (lastChar == '*') {
                    lastChar = this->getchar();
                    if (lastChar == '/')
                        break;
                }
            }
            return getNextToken();
        } 
    }

	this->currentToken = lastChar;
    return lastChar;
}

void Lexer::ungetCurrentToken() {
	this->currentIndex--;
}

int16_t Lexer::getCurrentToken() {
    return this->currentToken;
}

int Lexer::getIntegerValue() {
	return this->integerValue;
}

float Lexer::getFloatValue() {
	return this->floatValue;
}

bool Lexer::getBoolValue() {
    return this->boolValue;
}

char Lexer::getCharValue() {
    return this->charValue;
}

std::string Lexer::getIdentifierName() {
    return this->identifierName;
}

Token Lexer::getKeywordToken(const std::string& identifier) {
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        return it->second;
    } else { 
        this->identifierName = identifier;
        return tok_identifier;              
    }
}
