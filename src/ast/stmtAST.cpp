#include "stmtAST.hpp"

#include "exprAST.hpp"
#include "../codegen/CodeGenerator.hpp"

llvm::Value *VarDeclStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *AssignmentStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *IfStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *WhileStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *BlockStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *PrintStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *IndexAssignStmtAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}
