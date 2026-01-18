cmake_minimum_required(VERSION 3.23)

project(BluePrint LANGUAGES CXX)

add_executable(BluePrint
    src/main.cpp
	src/lexer/lexer.cpp
	src/parser/parser.cpp
	src/parser/valueParsers.cpp
	src/parser/classParser.cpp
	src/parser/statementParser.cpp
)
