cmake_minimum_required(VERSION 3.23)

project(BluePrint LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(BluePrint
    src/main.cpp
	src/lexer/lexer.cpp
	src/parser/parser.cpp
	src/parser/valueParsers.cpp
    src/parser/expressionparser.cpp
	src/parser/classParser.cpp
	src/parser/statementParser.cpp
	src/ast/exprAST.cpp
    src/ast/stmtAST.cpp
    src/ast/classAST.cpp
	src/codegen/CodeGenerator.cpp
)

# Link against LLVM libraries
if (TARGET LLVM)
    target_link_libraries(BluePrint PRIVATE LLVM)
else()
    llvm_map_components_to_libnames(llvm_libs 
        Core
        Support
        Target
        MC
        MCParser
        CodeGen
        IRReader
        ExecutionEngine
        Object
        OrcJIT
        RuntimeDyld
        native
        nativecodegen
    )

    target_link_libraries(BluePrint PRIVATE ${llvm_libs})
endif()
