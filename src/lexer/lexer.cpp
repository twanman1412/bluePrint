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

		do {
			str += firstChar;
			firstChar = getchar();
		} while (isalnum(firstChar));
		std::cout << "Token parsed: " << str << std::endl;

		if (str == "int") {
			std::cout << "Token is token for 'int'" << std::endl;
			return bp_lexer::bp_tok_int;
		}

		if (str == "float") {
			std::cout << "Token is token for 'float'" << std::endl;
			return bp_lexer::bp_tok_float;
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
		std::cout << "Token parsed: " << str << std::endl;

		if (isFloat) {
			std::cout << "Token is a float value" << std::endl;
			floatNum = strtod(str.c_str(), 0);
			return 0;
		} else {
			std::cout << "Token is a int value" << std::endl;
			num = stoi(str);
			return 0;
		}

		std::cerr << "Token is neither float nor integer" << std::endl;
		return 0;
	}

	std::cout << "Token starts with special character" << std::endl;
	return 0;
}
