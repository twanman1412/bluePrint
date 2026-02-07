#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vector>

class ExprAST {
    public:
        virtual ~ExprAST() = default;
};

class IntegerExprAST : public ExprAST {
    public:
        IntegerExprAST(long long value) : value(value) {}
        long long getValue() const { return value; }
    private:
        long long value;
};

class FloatExprAST : public ExprAST {
    public:
        FloatExprAST(double value) : value(value) {}
        double getValue() const { return value; }
    private:
        double value;
};

class BoolExprAST : public ExprAST {
    public:
        BoolExprAST(bool value) : value(value) {}
        bool getValue() const { return value; }
    private:
        bool value;
};

class CharExprAST : public ExprAST {
    public:
        CharExprAST(char value) : value(value) {}
        char getValue() const { return value; }
    private:
        char value;
};

class IdentifierExprAST : public ExprAST {
    public:
        IdentifierExprAST(const std::string &name) : name(name) {}
        const std::string &getName() const { return name; }
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
		
	private:
		int op;
		std::unique_ptr<ExprAST> operand;
};

