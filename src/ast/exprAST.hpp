#pragma once

#include "commonAST.hpp"
#include <string>
#include <memory>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenerator;

class ExprAST {
    public:
        virtual ~ExprAST() = default;
		virtual llvm::Value *codegen(CodeGenerator& generator) = 0;
};

class IntegerExprAST : public ExprAST {
    public:
        IntegerExprAST(long long value) : value(value) {}
        long long getValue() const { return value; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        long long value;
};

class FloatExprAST : public ExprAST {
    public:
        FloatExprAST(double value) : value(value) {}
        double getValue() const { return value; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        double value;
};

class BoolExprAST : public ExprAST {
    public:
        BoolExprAST(bool value) : value(value) {}
        bool getValue() const { return value; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        bool value;
};

class CharExprAST : public ExprAST {
    public:
        CharExprAST(char value) : value(value) {}
        char getValue() const { return value; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        char value;
};

class StrExprAST : public ExprAST {
    public:
        StrExprAST(const std::string& value) : value(value) {}
        const std::string& getValue() const { return value; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        std::string value;
};

class ArrayLiteralExprAST : public ExprAST {
    public:
        ArrayLiteralExprAST(std::vector<std::unique_ptr<ExprAST>> elements)
            : elements(std::move(elements)) {}
        const std::vector<std::unique_ptr<ExprAST>>& getElements() const { return elements; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        std::vector<std::unique_ptr<ExprAST>> elements;
};

class ArrayNewExprAST : public ExprAST {
    public:
        ArrayNewExprAST(std::unique_ptr<TypeAST> elementType, std::unique_ptr<ExprAST> size)
            : elementType(std::move(elementType)), size(std::move(size)) {}
        const TypeAST* getElementType() const { return elementType.get(); }
        ExprAST* getSize() const { return size.get(); }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        std::unique_ptr<TypeAST> elementType;
        std::unique_ptr<ExprAST> size;
};

class IndexExprAST : public ExprAST {
    public:
        IndexExprAST(const std::string& name, std::unique_ptr<ExprAST> index)
            : name(name), index(std::move(index)) {}
        const std::string& getName() const { return name; }
        ExprAST* getIndex() const { return index.get(); }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        std::string name;
        std::unique_ptr<ExprAST> index;
};

class IdentifierExprAST : public ExprAST {
    public:
        IdentifierExprAST(const std::string &name) : name(name) {}
        const std::string &getName() const { return name; }
        llvm::Value *codegen(CodeGenerator& generator) override;
    private:
        std::string name;
};

class BinaryExprAST : public ExprAST {
    public:

		enum Operator {
			PLUS,
			MINUS,
			MULTIPLY,
			DIVIDE,
			MODULO,
			EQUAL,
			NOT_EQUAL,
			LESS_THAN,
			LESS_EQUAL,
			GREATER_THAN,
			GREATER_EQUAL,
			LOGICAL_AND,
			LOGICAL_OR,
		};


        BinaryExprAST(int op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        
        int getOp() const { return op; }
        ExprAST *getLHS() const { return lhs.get(); }
        ExprAST *getRHS() const { return rhs.get(); }
        llvm::Value *codegen(CodeGenerator& generator) override;
        
    private:
        int op;
        std::unique_ptr<ExprAST> lhs;
        std::unique_ptr<ExprAST> rhs;
};

class UnaryExprAST : public ExprAST {
	public:
		enum Operator {
			NEGATE,
			LOGICAL_NOT,
		};

		UnaryExprAST(int op, std::unique_ptr<ExprAST> operand)
			: op(op), operand(std::move(operand)) {}
		
		int getOp() const { return op; }
		ExprAST *getOperand() const { return operand.get(); }
		llvm::Value *codegen(CodeGenerator& generator) override;
		
	private:
		int op;
		std::unique_ptr<ExprAST> operand;
};

