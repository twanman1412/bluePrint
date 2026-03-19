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

// StrExprAST codegen - delegates to visitor
llvm::Value *StrExprAST::codegen(CodeGenerator& generator) {
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

// ArrayLiteralExprAST codegen - delegates to visitor
llvm::Value *ArrayLiteralExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// ArrayNewExprAST codegen - delegates to visitor
llvm::Value *ArrayNewExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

// IndexExprAST codegen - delegates to visitor
llvm::Value *IndexExprAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}