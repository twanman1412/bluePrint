#pragma once

#include <string>
#include <memory>

class TypeAST {
    public:
        virtual ~TypeAST() = default;
};

class PrimitiveTypeAST : public TypeAST {
    public:
        enum PrimitiveKind {
            INT8,
            INT16,
            INT32,
            INT64,
            UINT8,
            UINT16,
            UINT32,
            UINT64,
            FLOAT32,
            FLOAT64,
            FRACTIONAL32,
            FRACTIONAL64,
            BOOL,
            CHAR,
            STR,
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
