#pragma once

#include <memory>

#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

class Parser {
    public:
        Parser(Lexer lexer) : lexer(std::move(lexer)) {}
        ~Parser() = default;

		void parse();

		std::unique_ptr<ExprAST> parseExpression();
        std::unique_ptr<ExprAST> parseParenExpression();
        std::unique_ptr<BinaryExprAST> parseBinaryExpression();
		std::unique_ptr<ExprAST> parseBinaryOpRHS(int exprPrecedence, std::unique_ptr<ExprAST> lhs);

		std::unique_ptr<ExprAST> parsePrimaryExpression();
        std::unique_ptr<IntegerExprAST> parseIntegerValue();
        std::unique_ptr<FloatExprAST> parseFloatValue();
        std::unique_ptr<BoolExprAST> parseBoolValue();
        std::unique_ptr<CharExprAST> parseCharValue();

        std::unique_ptr<IdentifierExprAST> parseIdentifier();

		std::unique_ptr<ClassAST> parseClassDefinition();
		std::unique_ptr<MethodImplAST> parseMethodImplementation();
		std::unique_ptr<StmtAST> parseStatement();

    private:
        Lexer lexer;
};

namespace ParserUtils {

	inline static std::unique_ptr<TypeAST> getPrimitiveTypeFromToken(int16_t token) {
		switch (token) {
			case tok_i32:
				return std::make_unique<PrimitiveTypeAST>(PrimitiveTypeAST::INT32);
			case tok_f32:
				return std::make_unique<PrimitiveTypeAST>(PrimitiveTypeAST::FLOAT32);
			case tok_bool:
				return std::make_unique<PrimitiveTypeAST>(PrimitiveTypeAST::BOOL);
			case tok_char:
				return std::make_unique<PrimitiveTypeAST>(PrimitiveTypeAST::CHAR);
			case tok_void:
				return std::make_unique<PrimitiveTypeAST>(PrimitiveTypeAST::VOID);
			default:
				return nullptr;
		}
	}

	inline static std::unique_ptr<TypedIdentifierAST> makeTypedIdentifier(std::unique_ptr<TypeAST> type, const std::string &name) {
		return std::make_unique<TypedIdentifierAST>(std::move(type), name);
	}

	inline static std::unique_ptr<AccessModifierAST> getAccessModifierFromToken(int16_t token) {
		switch (token) {
			case tok_public:
				return std::make_unique<AccessModifierAST>(AccessModifierAST::PUBLIC);
			default:
				return nullptr;
		}
	}
}

