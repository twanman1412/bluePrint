#include "exprAST.hpp"
#include "../codegen/CodeGenerator.hpp"

// IntegerExprAST codegen - delegates to visitor
llvm::Value *IntegerExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// FloatExprAST codegen - delegates to visitor
llvm::Value *FloatExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// BoolExprAST codegen - delegates to visitor
llvm::Value *BoolExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// CharExprAST codegen - delegates to visitor
llvm::Value *CharExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// IdentifierExprAST codegen - delegates to visitor
llvm::Value *IdentifierExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// BinaryExprAST codegen - delegates to visitor
llvm::Value *BinaryExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// UnaryExprAST codegen - delegates to visitor
llvm::Value *UnaryExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}