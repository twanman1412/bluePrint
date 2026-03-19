#include <iostream>
#include <unordered_map>
#include <cctype>

#include "lexer.hpp"
#include "tokens.hpp"

// Initialize the keyword lookup table
std::unordered_map<std::string, Token> Lexer::keywords = {
    // Primitive types
    {"i8", tok_i8},
    {"i16", tok_i16},
    {"i32", tok_i32},
    {"i64", tok_i64},
    {"u8", tok_u8},
    {"u16", tok_u16},
    {"u32", tok_u32},
    {"u64", tok_u64},
    {"f32", tok_f32},
    {"f64", tok_f64},
    {"fr32", tok_fr32},
    {"fr64", tok_fr64},
    {"bool", tok_bool}, 
	{"char", tok_char}, 
	{"void", tok_void},
    {"str",  tok_str},
    
    // Literals
    {"true", tok_true}, 
	{"false", tok_false},
    
    // Declaration keywords
    {"class", tok_class},
    {"new",   tok_new},
    
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
		if (this->currentToken == tok_true) {
			this->boolValue = true;
		} else if (this->currentToken == tok_false) {
			this->boolValue = false;
		}
		return this->currentToken;
    }

    const bool dotStartsFloat = lastChar == '.' && currentIndex < source.size() && isdigit(source[currentIndex]);
    if (isdigit(lastChar) || dotStartsFloat) {
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

		ungetCurrentToken();

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
        char nextChar = this->getchar();
        if (nextChar == '/') {
            // Single line comment
            do {
                lastChar = this->getchar();
            } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

            if (lastChar != EOF)
                return getNextToken();
        } else if (nextChar == '*') {
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

		ungetCurrentToken();
		this->currentToken = '/';
		return this->currentToken;
    }

    if (lastChar == '\'') {
        char value = this->getchar();
        char closing = this->getchar();
        if (value == EOF || closing != '\'') {
            std::cerr << "Error: Invalid char literal." << std::endl;
            return tok_eof;
        }

        this->charValue = value;
        this->currentToken = tok_char_literal;
        return this->currentToken;
    }

    if (lastChar == '=') {
        char nextChar = this->getchar();
        if (nextChar == '=') {
            this->currentToken = tok_equal_equal;
            return this->currentToken;
        }
        ungetCurrentToken();
    }

    if (lastChar == '!') {
        char nextChar = this->getchar();
        if (nextChar == '=') {
            this->currentToken = tok_not_equal;
            return this->currentToken;
        }
        ungetCurrentToken();
    }

    if (lastChar == '<') {
        char nextChar = this->getchar();
        if (nextChar == '=') {
            this->currentToken = tok_less_equal;
            return this->currentToken;
        }
        ungetCurrentToken();
    }

    if (lastChar == '>') {
        char nextChar = this->getchar();
        if (nextChar == '=') {
            this->currentToken = tok_greater_equal;
            return this->currentToken;
        }
        ungetCurrentToken();
    }

    if (lastChar == '&') {
        char nextChar = this->getchar();
        if (nextChar == '&') {
            this->currentToken = '&';
            return this->currentToken;
        }
        ungetCurrentToken();
    }

    if (lastChar == '|') {
        char nextChar = this->getchar();
        if (nextChar == '|') {
            this->currentToken = '|';
            return this->currentToken;
        }
        ungetCurrentToken();
    }

    if (lastChar == '"') {
        std::string strContent;
        while (true) {
            char c = this->getchar();
            if (c == EOF || c == '\n') {
                std::cerr << "Error: Unterminated string literal." << std::endl;
                return tok_eof;
            }
            if (c == '"') break;
            if (c == '\\') {
                char escaped = this->getchar();
                switch (escaped) {
                    case '"':  strContent += '"';  break;
                    case '\\': strContent += '\\'; break;
                    case 'n':  strContent += '\n'; break;
                    case 't':  strContent += '\t'; break;
                    case 'r':  strContent += '\r'; break;
                    default:
                        strContent += '\\';
                        strContent += escaped;
                        break;
                }
            } else {
                strContent += c;
            }
        }
        this->stringValue = strContent;
        this->currentToken = tok_str_literal;
        return this->currentToken;
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

int64_t Lexer::getIntegerValue() {
	return this->integerValue;
}

double Lexer::getFloatValue() {
	return this->floatValue;
}

bool Lexer::getBoolValue() {
    return this->boolValue;
}

char Lexer::getCharValue() {
    return this->charValue;
}

std::string Lexer::getStringValue() {
    return this->stringValue;
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
