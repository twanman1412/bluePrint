#include <string>

class bp_lexer {

public:
	enum bp_tokens {

		//NOTE: Control characters

		// End Of File and End Of Line
		bp_tok_eof = -1,
		bp_tok_eol = -2,

		//NOTE: Types

		// Integers
		bp_tok_int = -3,
		bp_tok_int_value = -4,

		// Floating points
		bp_tok_float = -5,
		bp_tok_double = -6,
		bp_tok_float_value = -7,

		// Long
		bp_tok_long = -8,
		bp_tok_long_value = -9,

		// Char
		bp_tok_char = -10,
		bp_tok_char_value = -11,

		// Bool
		bp_tok_bool = -12,
		bp_tok_bool_value = -13,

		// Void
		bp_tok_void = -14,

		//NOTE: Classe
		bp_tok_class = -50,
		bp_tok_class_name = -51,

		//NOTE: Variables

		//NOTE: Change to the correct value when all primitive types are determined
		bp_tok_var = -100,
	};
};
