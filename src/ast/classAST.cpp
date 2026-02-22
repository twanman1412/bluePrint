#include "classAST.hpp"

#include "../codegen/CodeGenerator.hpp"

llvm::Value *MethodImplAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}

llvm::Value *ClassAST::codegen(CodeGenerator& generator) {
    return generator.visit(*this);
}
