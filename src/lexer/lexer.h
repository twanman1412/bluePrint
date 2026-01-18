#pragma once

#include <string>
#include <unordered_map>

#include "tokens.h"

class Lexer {

    public:
        Lexer(std::string sourceCode);
        ~Lexer() = default;

        // Token management
        int16_t getNextToken();
		void ungetCurrentToken();
        int16_t getCurrentToken();

        // Value retrieval based on token type
        int getIntegerValue();        // For all integer literals
        float getFloatValue();             // For all floating-point literals
        bool getBoolValue();
        char getCharValue();

        // Identifier retrieval
        std::string getIdentifierName();

    private:
		// Source code to be tokenized
		std::string source;
		size_t currentIndex = 0;
		char getchar();

        // Token state 
        int16_t currentToken;

        // Value storage for different token types
        int integerValue;             // For all integer literals
        float floatValue;                  // For all floating-point literals
        bool boolValue;
        char charValue;

        // Identifier storage
        std::string identifierName;
        
        // Keyword lookup table
        static std::unordered_map<std::string, Token> keywords;
        
        // Helper methods
        Token getKeywordToken(const std::string& identifier);
};
