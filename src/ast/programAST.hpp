#pragma once

#include <llvm/IR/Value.h>

class CodeGenerator;

class ProgramAST {
    public:
        virtual ~ProgramAST() = default;
    virtual llvm::Value *codegen(CodeGenerator& generator) = 0;
};

