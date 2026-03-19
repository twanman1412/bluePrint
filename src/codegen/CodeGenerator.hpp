#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <memory>
#include <map>
#include <string>
#include <optional>

#include "../ast/exprAST.hpp"
#include "../ast/stmtAST.hpp"
#include "../ast/classAST.hpp"
#include "../ast/commonAST.hpp"

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
    std::map<std::string, PrimitiveTypeAST::PrimitiveKind> NamedPrimitiveKinds;
    std::map<const llvm::Value*, PrimitiveTypeAST::PrimitiveKind> ValuePrimitiveKinds;
    std::optional<unsigned> ExpectedIntegerResultBits;
    std::optional<PrimitiveTypeAST::PrimitiveKind> ExpectedFractionResultKind;
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
    const PrimitiveTypeAST* getPrimitiveType(const TypeAST* typeAST) const;
    bool isUnsignedPrimitiveKind(PrimitiveTypeAST::PrimitiveKind kind) const;
    void setNamedPrimitiveKind(const std::string& name, PrimitiveTypeAST::PrimitiveKind kind);
    bool getNamedPrimitiveKind(const std::string& name, PrimitiveTypeAST::PrimitiveKind& outKind) const;
    void setValuePrimitiveKind(llvm::Value* value, PrimitiveTypeAST::PrimitiveKind kind);
    bool getValuePrimitiveKind(llvm::Value* value, PrimitiveTypeAST::PrimitiveKind& outKind) const;
    bool isUnsignedValue(llvm::Value* value) const;
    PrimitiveTypeAST::PrimitiveKind getIntegerPrimitiveKind(unsigned bitWidth, bool isUnsigned) const;
    bool isFractionalPrimitiveKind(PrimitiveTypeAST::PrimitiveKind kind) const;
    bool isFractionalValue(llvm::Value* value) const;
    unsigned getFractionalComponentBitWidth(PrimitiveTypeAST::PrimitiveKind kind) const;
    PrimitiveTypeAST::PrimitiveKind getFractionalPrimitiveKindForType(llvm::Type* type) const;
    llvm::Type* getFractionalComponentType(PrimitiveTypeAST::PrimitiveKind kind);
    llvm::StructType* getFractionalLLVMType(PrimitiveTypeAST::PrimitiveKind kind);
    llvm::Function* getOrCreateGcdFunction(unsigned bitWidth);
    llvm::Value* buildFractionValue(llvm::Value* numerator, llvm::Value* denominator, PrimitiveTypeAST::PrimitiveKind kind);
    bool decomposeFractionValue(llvm::Value* fractionValue, PrimitiveTypeAST::PrimitiveKind kind, llvm::Value*& numerator, llvm::Value*& denominator);
    llvm::Value* castIntegerToFraction(llvm::Value* value, PrimitiveTypeAST::PrimitiveKind targetKind);
    llvm::Value* castFractionToFloatingPoint(llvm::Value* value, llvm::Type* targetType);
    llvm::Value* createFractionArithmetic(int op, llvm::Value* leftValue, llvm::Value* rightValue);
    llvm::Value* createFractionComparison(int op, llvm::Value* leftValue, llvm::Value* rightValue);
};
