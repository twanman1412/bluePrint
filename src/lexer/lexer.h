#include <string>

class bp_lexer {

public:
	enum bp_tokens {

		bp_tok_eof = -1,
		bp_tok_eol = -2,

		bp_tok_int = -3,
		bp_tok_float = -4,


		//NOTE: Change to the correct value when all primitive types are determined
		bp_tok_var = -5,
	};
};
