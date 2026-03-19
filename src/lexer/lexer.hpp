#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

#include "tokens.hpp"

class Lexer {

    public:
        Lexer(std::string sourceCode);
        ~Lexer() = default;

        // Token management
        int16_t getNextToken();
		void ungetCurrentToken();
        int16_t getCurrentToken();

        // Value retrieval based on token type
        int64_t getIntegerValue();
        double getFloatValue();
        bool getBoolValue();
        char getCharValue();
        std::string getStringValue();

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
        int64_t integerValue;
        double floatValue;
        bool boolValue;
        char charValue;
        std::string stringValue;

        // Identifier storage
        std::string identifierName;
        
        // Keyword lookup table
        static std::unordered_map<std::string, Token> keywords;
        
        // Helper methods
        Token getKeywordToken(const std::string& identifier);
};
