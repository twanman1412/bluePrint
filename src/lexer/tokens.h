#pragma once

#include <array>
#include <cstdint>

enum Token : int16_t {
    // End of file
    tok_eof = -1,

    // Primitive types 
    tok_i32 = -10, 
    tok_f32 = -11, 
    tok_bool = -12, 
    tok_char = -13, 
	tok_void = -14,
    
    // Literals 
    tok_true = -50, 
    tok_false = -51, 
    tok_integer_literal = -52,
    tok_float_literal = -53,
    tok_char_literal = -54,
    
    // Identifiers 
    tok_identifier = -100,
    
    // Declaration keywords 
    tok_class = -150, 

    // Control flow 
    tok_if = -200, 
    tok_else = -201, 
    tok_while = -202, 

	// Access modifiers
	tok_public = -250,
};

namespace TokenUtils {
	inline std::array<Token, 5> getPrimitiveTypeTokens() {
		return {tok_i32, tok_f32, tok_bool, tok_char, tok_void};
	}

	inline static bool isPrimitiveTypeToken(int16_t token) {
		for (const auto &primToken : getPrimitiveTypeTokens()) {
			if (token == primToken) {
				return true;
			}
		}
		return false;
	}

	inline static bool isPrimitiveTypeToken(Token token) {
		for (const auto &primToken : getPrimitiveTypeTokens()) {
			if (token == primToken) {
				return true;
			}
		}
		return false;
	}

	inline std::array<Token, 5> getLiteralTokens() {
		return {tok_integer_literal, tok_float_literal, tok_char_literal, tok_true, tok_false};
	}

	inline static bool isLiteralToken(int16_t token) {
		for (const auto &litToken : getLiteralTokens()) {
			if (token == litToken) {
				return true;
			}
		}
		return false;
	}

	inline static bool isLiteralToken(Token token) {
		for (const auto &litToken : getLiteralTokens()) {
			if (token == litToken) {
				return true;
			}
		}
		return false;
	}

	inline std::array<Token, 3> getControlFlowTokens() {
		return {tok_if, tok_else, tok_while};
	}

	inline static bool isControlFlowToken(int16_t token) {
		for (const auto &cfToken : getControlFlowTokens()) {
			if (token == cfToken) {
				return true;
			}
		}
		return false;
	}

	inline static bool isControlFlowToken(Token token) {
		for (const auto &cfToken : getControlFlowTokens()) {
			if (token == cfToken) {
				return true;
			}
		}
		return false;
	}

	inline std::array<Token, 1> getAccessModifierTokens() {
		return {tok_public};
	}

	inline static bool isAccessModifierToken(int16_t token) {
		for (const auto &amToken : getAccessModifierTokens()) {
			if (token == amToken) {
				return true;
			}
		}
		return false;
	}

	inline static bool isAccessModifierToken(Token token) {
		for (const auto &amToken : getAccessModifierTokens()) {
			if (token == amToken) {
				return true;
			}
		}
		return false;
	}
}

