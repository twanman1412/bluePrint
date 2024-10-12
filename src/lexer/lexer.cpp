#include <cctype>
#include <cstdio>
#include <iostream>
#include <stdlib.h>

#include "lexer.h"

int getToken();

int main(int argc, char *argv[]) {

	std::string *str;
	int *num;
	float *floatNum;

	while (getToken() != -1);
}

int getToken() {

	std::string str;
	int num;
	float floatNum;

	char firstChar = ' ';
	while (isspace(firstChar)) {
		firstChar = getchar();
	}

	if (isalpha(firstChar)) {

		bool isClassName = isupper(firstChar);


		do {
			str += firstChar;
			firstChar = getchar();
		} while (isalnum(firstChar));

		if (isClassName) {
			std::cout << "Token is token for classname" << std::endl;
			return bp_lexer::bp_tok_class_name;
		}

		if (str == "int") {
			std::cout << "Token is token for 'int'" << std::endl;
			return bp_lexer::bp_tok_int;
		}

		if (str == "float") {
			std::cout << "Token is token for 'float'" << std::endl;
			return bp_lexer::bp_tok_float;
		}

		if (str == "double") {
			std::cout << "Token is token for 'double'" << std::endl;
			return bp_lexer::bp_tok_double;
		}

		if (str == "char") {
			std::cout << "Token is token for 'char'" << std::endl;
			return bp_lexer::bp_tok_char;
		}

		if (str == "bool") {
			std::cout << "Token is token for 'bool'" << std::endl;
			return bp_lexer::bp_tok_bool;
		}

		if (str == "void") {
			std::cout << "Token is token for 'void'" << std::endl;
			return bp_lexer::bp_tok_void;
		}

		if (str == "class") {
			std::cout << "Token is token for 'class'" << std::endl;
			return bp_lexer::bp_tok_class;
		}

		if (str == "true" || str == "false") {
			std::cout << "Token is boolean value" << std::endl;
			return bp_lexer::bp_tok_bool_value;
		}

		std::cout << "Token is a variable name" << std::endl;
		return bp_lexer::bp_tok_var;
	}

	if (isdigit(firstChar) || firstChar == '.') {

		bool isFloat = firstChar == '.';

		do {
			str += firstChar;
			firstChar = getchar();

			if (firstChar == '.') {

				if (isFloat) {
					std::cerr << "Error when parsing number, second '.' found" << std::endl;
					return 0;
				}

				isFloat = true;
			}
		} while (isdigit(firstChar) || firstChar == '.');

		if (isFloat) {
			std::cout << "Token is a float value" << std::endl;
			floatNum = strtof(str.c_str(), 0);
			return bp_lexer::bp_tok_float_value;
		} else {
			std::cout << "Token is a int value" << std::endl;
			num = stoi(str);
			return bp_lexer::bp_tok_int_value;
		}

		std::cerr << "Token is neither float nor integer" << std::endl;
		return 0;
	}

	if (firstChar == '#') {

		std::cout << "Comment, skipping line" << std::endl;
		do {
			firstChar = getchar();
		} while (firstChar != EOF && firstChar != '\n' && firstChar != '\r');
	}

	if (firstChar == '-') {

		firstChar = getchar();
		if (firstChar == '#') {

			std::cout << "Multi-line comment, skipping" << std::endl;
			do {
				firstChar = getchar();
				if (firstChar == '#') {

					firstChar = getchar();
					if (firstChar == '-') {
						return 0;
					}
				}
			} while (firstChar != EOF);
		}
	}
	std::cout << "Token starts with special character" << std::endl;
	return 0;
}
