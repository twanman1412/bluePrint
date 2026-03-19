#pragma once

#include <array>
#include <cstdint>

enum Token : int16_t {
    // End of file
    tok_eof = -1,

    // Primitive types 
	tok_i8 = -10,
	tok_i16 = -11,
	tok_i32 = -12,
	tok_i64 = -13,
	tok_u8 = -14,
	tok_u16 = -15,
	tok_u32 = -16,
	tok_u64 = -17,
	tok_f32 = -18,
	tok_f64 = -19,
	tok_fr32 = -20,
	tok_fr64 = -21,
	tok_bool = -22,
	tok_char = -23,
	tok_void = -24,
	tok_str  = -25,
    
    // Literals 
    tok_true = -50, 
    tok_false = -51, 
    tok_integer_literal = -52,
    tok_float_literal = -53,
    tok_char_literal = -54,
    tok_str_literal  = -55,
    
    // Identifiers 
    tok_identifier = -100,
    
    // Declaration keywords 
    tok_class = -150, 
	tok_new   = -151,

    // Control flow 
    tok_if = -200, 
    tok_else = -201, 
    tok_while = -202, 

	// Access modifiers
	tok_public = -250,

	// Operators
	tok_equal_equal = -300,
	tok_not_equal = -301,
	tok_less_equal = -302,
	tok_greater_equal = -303,
};

namespace TokenUtils {
	inline std::array<Token, 16> getPrimitiveTypeTokens() {
		return {tok_i8, tok_i16, tok_i32, tok_i64, tok_u8, tok_u16, tok_u32, tok_u64, tok_f32, tok_f64, tok_fr32, tok_fr64, tok_bool, tok_char, tok_void, tok_str};
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

	inline std::array<Token, 6> getLiteralTokens() {
		return {tok_integer_literal, tok_float_literal, tok_char_literal, tok_str_literal, tok_true, tok_false};
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

