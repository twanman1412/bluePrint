#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <memory>
#include <map>
#include <string>

#include "../ast/exprAST.hpp"
#include "../ast/stmtAST.hpp"
#include "../ast/classAST.hpp"

class CodeGenerator {
public:
    CodeGenerator();
    ~CodeGenerator() = default;

    // Visitor methods for expression AST nodes
    llvm::Value* visit(IntegerExprAST& node);
    llvm::Value* visit(FloatExprAST& node);
    llvm::Value* visit(BoolExprAST& node);
    llvm::Value* visit(CharExprAST& node);
    llvm::Value* visit(IdentifierExprAST& node);
    llvm::Value* visit(BinaryExprAST& node);
    llvm::Value* visit(UnaryExprAST& node);

    // Visitor methods for statement AST nodes
    llvm::Value* visit(VarDeclStmtAST& node);
    llvm::Value* visit(AssignmentStmtAST& node);
    llvm::Value* visit(IfStmtAST& node);
    llvm::Value* visit(WhileStmtAST& node);
    llvm::Value* visit(BlockStmtAST& node);
    llvm::Value* visit(PrintStmtAST& node);

    // Visitor methods for class/program AST nodes
    llvm::Value* visit(MethodImplAST& node);
    llvm::Value* visit(ClassAST& node);

    // Utility methods
    llvm::LLVMContext& getContext() { return TheContext; }
    llvm::IRBuilder<>& getBuilder() { return Builder; }
    llvm::Module* getModule() { return TheModule.get(); }
    std::unique_ptr<llvm::Module> takeModule();
    void dumpIR() const;
    
    void setNamedValue(const std::string& name, llvm::AllocaInst* value);
    llvm::AllocaInst* getNamedValue(const std::string& name);

private:
    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> TheModule;
    std::map<std::string, llvm::AllocaInst*> NamedValues;
    llvm::Function* CurrentFunction;
    std::string CurrentClassName;
    bool CurrentClassIsApplication;

    // Helper function for logging errors
    llvm::Value* logError(const char* str);
    llvm::Type* getLLVMType(const TypeAST* typeAST);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, llvm::Type* type, const std::string& name);
    llvm::Value* castValueToType(llvm::Value* value, llvm::Type* targetType);
    llvm::Value* castToBoolean(llvm::Value* value);
    llvm::FunctionCallee getPrintfFunction();
};
