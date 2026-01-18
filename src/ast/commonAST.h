#pragma once

#include <string>
#include <memory>
#include <vector>

class TypeAST {
    public:
        virtual ~TypeAST() = default;
};

class PrimitiveTypeAST : public TypeAST {
    public:
        enum PrimitiveKind {
            INT32,
            FLOAT32,
            BOOL,
            CHAR,
			VOID,
        };

		PrimitiveTypeAST() = default;
        PrimitiveTypeAST(PrimitiveKind kind) : kind(kind) {}
        PrimitiveKind getKind() const { return kind; }

    private:
		PrimitiveKind kind;
};

class TypedIdentifierAST {
	public:
		TypedIdentifierAST(std::unique_ptr<TypeAST> type, const std::string &name)
			: type(std::move(type)), name(name) {}

		const TypeAST *getType() const { return type.get(); }
		const std::string &getName() const { return name; }

	private:
		std::unique_ptr<TypeAST> type;
		std::string name;
};
