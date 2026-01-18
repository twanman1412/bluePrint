#pragma once

#include <string>
#include <vector>
#include <memory>

#include "exprAST.h"
#include "programAST.h"
#include "stmtAST.h"
#include "commonAST.h"

class AccessModifierAST {
	public: 
		enum AccessModifierKind {
			PUBLIC,
		};

		AccessModifierAST(AccessModifierKind kind) : kind(kind) {}
		AccessModifierKind getKind() const { return kind; }

	private:
		AccessModifierKind kind;
};

class MethodImplAST {
	public:
		MethodImplAST() = default;
		MethodImplAST(
				std::vector<std::unique_ptr<AccessModifierAST>> accessModifiers,
				std::unique_ptr<TypeAST> returnType, 
				const std::string &name, 
				std::vector<std::unique_ptr<TypedIdentifierAST>> params, 
				std::vector<std::unique_ptr<StmtAST>> body)
				: accessModifiers(std::move(accessModifiers)), returnType(std::move(returnType)), name(name), params(std::move(params)), body(std::move(body)) {}

		const std::vector<std::unique_ptr<AccessModifierAST>> &getAccessModifiers() const { return accessModifiers; }
		const TypeAST *getReturnType() const { return returnType.get(); }
		const std::string &getName() const { return name; }
		const std::vector<std::unique_ptr<TypedIdentifierAST>> &getParams() const { return params; }
		const std::vector<std::unique_ptr<StmtAST>> &getBody() const { return body; }

	private:
		std::vector<std::unique_ptr<AccessModifierAST>> accessModifiers;
		std::unique_ptr<TypeAST> returnType;
		std::string name;
		std::vector<std::unique_ptr<TypedIdentifierAST>> params;
		std::vector<std::unique_ptr<StmtAST>> body;
};

class ClassAST : public ProgramAST {
	public:
		ClassAST(const std::string &name, 
				std::vector<std::unique_ptr<MethodImplAST>> methodImpls, 
				std::vector<std::string> blueprintNames = {})
			: name(name), methodImpls(std::move(methodImpls)), blueprintNames(std::move(blueprintNames)) {}

		const std::string &getName() const { return name; }
		const std::vector<std::unique_ptr<MethodImplAST>> &getMethodImpls() const { return methodImpls; }
		const std::vector<std::string> &getBlueprintNames() const { return blueprintNames; }

	private:
		std::string name;
		std::vector<std::unique_ptr<MethodImplAST>> methodImpls;
		std::vector<std::string> blueprintNames;
};

