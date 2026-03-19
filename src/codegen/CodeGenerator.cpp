#include <llvm/IR/Constants.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <cstdio>
#include <algorithm>

#include "CodeGenerator.hpp"
#include "../ast/exprAST.hpp"
#include "../ast/stmtAST.hpp"
#include "../ast/commonAST.hpp"

CodeGenerator::CodeGenerator()
    : TheContext(), Builder(TheContext), TheModule(nullptr), NamedValues(), NamedPrimitiveKinds(), ValuePrimitiveKinds(), ExpectedIntegerResultBits(std::nullopt), CurrentFunction(nullptr), CurrentClassName(), CurrentClassIsApplication(false) {
    TheModule = std::make_unique<llvm::Module>("BluePrint", TheContext);
}

llvm::Value* CodeGenerator::logError(const char* str) {
    fprintf(stderr, "CodeGen Error: %s\n", str);
    return nullptr;
}

void CodeGenerator::dumpIR() const {
    TheModule->print(llvm::outs(), nullptr);
}

std::unique_ptr<llvm::Module> CodeGenerator::takeModule() {
    return std::move(TheModule);
}

void CodeGenerator::setNamedValue(const std::string& name, llvm::AllocaInst* value) {
    NamedValues[name] = value;
}

void CodeGenerator::setNamedPrimitiveKind(const std::string& name, PrimitiveTypeAST::PrimitiveKind kind) {
    NamedPrimitiveKinds[name] = kind;
}

bool CodeGenerator::getNamedPrimitiveKind(const std::string& name, PrimitiveTypeAST::PrimitiveKind& outKind) const {
    auto it = NamedPrimitiveKinds.find(name);
    if (it == NamedPrimitiveKinds.end()) {
        return false;
    }

    outKind = it->second;
    return true;
}

void CodeGenerator::setValuePrimitiveKind(llvm::Value* value, PrimitiveTypeAST::PrimitiveKind kind) {
    if (value) {
        ValuePrimitiveKinds[value] = kind;
    }
}

bool CodeGenerator::getValuePrimitiveKind(llvm::Value* value, PrimitiveTypeAST::PrimitiveKind& outKind) const {
    if (!value) {
        return false;
    }

    auto it = ValuePrimitiveKinds.find(value);
    if (it == ValuePrimitiveKinds.end()) {
        return false;
    }

    outKind = it->second;
    return true;
}

const PrimitiveTypeAST* CodeGenerator::getPrimitiveType(const TypeAST* typeAST) const {
    return dynamic_cast<const PrimitiveTypeAST*>(typeAST);
}

bool CodeGenerator::isUnsignedPrimitiveKind(PrimitiveTypeAST::PrimitiveKind kind) const {
    return kind == PrimitiveTypeAST::UINT8 || kind == PrimitiveTypeAST::UINT16 || kind == PrimitiveTypeAST::UINT32 || kind == PrimitiveTypeAST::UINT64;
}

bool CodeGenerator::isUnsignedValue(llvm::Value* value) const {
    PrimitiveTypeAST::PrimitiveKind kind;
    return getValuePrimitiveKind(value, kind) && isUnsignedPrimitiveKind(kind);
}

PrimitiveTypeAST::PrimitiveKind CodeGenerator::getIntegerPrimitiveKind(unsigned bitWidth, bool isUnsigned) const {
    if (isUnsigned) {
        switch (bitWidth) {
            case 8:
                return PrimitiveTypeAST::UINT8;
            case 16:
                return PrimitiveTypeAST::UINT16;
            case 32:
                return PrimitiveTypeAST::UINT32;
            case 64:
            default:
                return PrimitiveTypeAST::UINT64;
        }
    }

    switch (bitWidth) {
        case 8:
            return PrimitiveTypeAST::INT8;
        case 16:
            return PrimitiveTypeAST::INT16;
        case 32:
            return PrimitiveTypeAST::INT32;
        case 64:
        default:
            return PrimitiveTypeAST::INT64;
    }
}

llvm::AllocaInst* CodeGenerator::getNamedValue(const std::string& name) {
    auto it = NamedValues.find(name);
    if (it != NamedValues.end()) {
        return it->second;
    }
    return nullptr;
}

llvm::Type* CodeGenerator::getLLVMType(const TypeAST* typeAST) {
    auto primitiveType = getPrimitiveType(typeAST);
    if (!primitiveType) {
        return nullptr;
    }

    switch (primitiveType->getKind()) {
        case PrimitiveTypeAST::INT8:
        case PrimitiveTypeAST::UINT8:
            return llvm::Type::getInt8Ty(TheContext);
        case PrimitiveTypeAST::INT16:
        case PrimitiveTypeAST::UINT16:
            return llvm::Type::getInt16Ty(TheContext);
        case PrimitiveTypeAST::INT32:
        case PrimitiveTypeAST::UINT32:
            return llvm::Type::getInt32Ty(TheContext);
        case PrimitiveTypeAST::INT64:
        case PrimitiveTypeAST::UINT64:
            return llvm::Type::getInt64Ty(TheContext);
        case PrimitiveTypeAST::FLOAT32:
            return llvm::Type::getFloatTy(TheContext);
        case PrimitiveTypeAST::FLOAT64:
            return llvm::Type::getDoubleTy(TheContext);
        case PrimitiveTypeAST::BOOL:
            return llvm::Type::getInt1Ty(TheContext);
        case PrimitiveTypeAST::CHAR:
            return llvm::Type::getInt8Ty(TheContext);
        case PrimitiveTypeAST::VOID:
            return llvm::Type::getVoidTy(TheContext);
        default:
            return nullptr;
    }
}

llvm::AllocaInst* CodeGenerator::createEntryBlockAlloca(llvm::Function* function, llvm::Type* type, const std::string& name) {
    llvm::IRBuilder<> tempBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tempBuilder.CreateAlloca(type, nullptr, name);
}

llvm::Value* CodeGenerator::castValueToType(llvm::Value* value, llvm::Type* targetType) {
    if (!value || !targetType) {
        return nullptr;
    }

    llvm::Type* sourceType = value->getType();
    if (sourceType == targetType) {
        return value;
    }

    if (sourceType->isIntegerTy() && targetType->isIntegerTy()) {
        const unsigned sourceBits = sourceType->getIntegerBitWidth();
        const unsigned targetBits = targetType->getIntegerBitWidth();
        PrimitiveTypeAST::PrimitiveKind sourceKind;
        const bool hasSourceKind = getValuePrimitiveKind(value, sourceKind);
        const bool sourceUnsigned = hasSourceKind && isUnsignedPrimitiveKind(sourceKind);
        if (sourceBits < targetBits) {
            return sourceUnsigned
                ? Builder.CreateZExt(value, targetType, "zexttmp")
                : Builder.CreateSExt(value, targetType, "sexttmp");
        }
        if (sourceBits > targetBits) {
            return Builder.CreateTrunc(value, targetType, "trunctmp");
        }
        return value;
    }

    if (sourceType->isIntegerTy() && targetType->isFloatingPointTy()) {
        return isUnsignedValue(value)
            ? Builder.CreateUIToFP(value, targetType, "uitofptmp")
            : Builder.CreateSIToFP(value, targetType, "sitofptmp");
    }

    if (sourceType->isFloatingPointTy() && targetType->isIntegerTy()) {
        return Builder.CreateFPToSI(value, targetType, "fptositmp");
    }

    if (sourceType->isFloatingPointTy() && targetType->isFloatingPointTy()) {
        if (sourceType->getPrimitiveSizeInBits() < targetType->getPrimitiveSizeInBits()) {
            return Builder.CreateFPExt(value, targetType, "fpexttmp");
        }
        return Builder.CreateFPTrunc(value, targetType, "fptrunctmp");
    }

    return nullptr;
}

llvm::Value* CodeGenerator::castToBoolean(llvm::Value* value) {
    if (!value) {
        return nullptr;
    }

    llvm::Type* type = value->getType();
    if (type->isIntegerTy(1)) {
        return value;
    }

    if (type->isIntegerTy()) {
        llvm::Value* zero = llvm::ConstantInt::get(type, 0);
        return Builder.CreateICmpNE(value, zero, "booltmp");
    }

    if (type->isFloatingPointTy()) {
        llvm::Value* zero = llvm::ConstantFP::get(type, 0.0);
        return Builder.CreateFCmpONE(value, zero, "booltmp");
    }

    return nullptr;
}

llvm::FunctionCallee CodeGenerator::getPrintfFunction() {
    llvm::Type* int8PointerType = llvm::PointerType::getUnqual(TheContext);
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(TheContext),
        {int8PointerType},
        true
    );
    return TheModule->getOrInsertFunction("printf", printfType);
}

llvm::Value* CodeGenerator::visit(IntegerExprAST& node) {
    llvm::Value* value = llvm::ConstantInt::get(TheContext, llvm::APInt(64, static_cast<uint64_t>(node.getValue()), false));
    setValuePrimitiveKind(value, PrimitiveTypeAST::INT64);
    return value;
}

llvm::Value* CodeGenerator::visit(FloatExprAST& node) {
    llvm::Value* value = llvm::ConstantFP::get(TheContext, llvm::APFloat(static_cast<double>(node.getValue())));
    setValuePrimitiveKind(value, PrimitiveTypeAST::FLOAT64);
    return value;
}

llvm::Value* CodeGenerator::visit(BoolExprAST& node) {
    llvm::Value* value = llvm::ConstantInt::get(TheContext, llvm::APInt(1, node.getValue() ? 1 : 0, false));
    setValuePrimitiveKind(value, PrimitiveTypeAST::BOOL);
    return value;
}

llvm::Value* CodeGenerator::visit(CharExprAST& node) {
    llvm::Value* value = llvm::ConstantInt::get(TheContext, llvm::APInt(8, static_cast<uint8_t>(node.getValue()), false));
    setValuePrimitiveKind(value, PrimitiveTypeAST::CHAR);
    return value;
}

llvm::Value* CodeGenerator::visit(IdentifierExprAST& node) {
    llvm::AllocaInst* alloca = getNamedValue(node.getName());
    if (!alloca) {
        return logError("Unknown variable name");
    }

    llvm::Value* loadedValue = Builder.CreateLoad(alloca->getAllocatedType(), alloca, node.getName() + ".val");
    PrimitiveTypeAST::PrimitiveKind primitiveKind;
    if (getNamedPrimitiveKind(node.getName(), primitiveKind)) {
        setValuePrimitiveKind(loadedValue, primitiveKind);
    }

    return loadedValue;
}

llvm::Value* CodeGenerator::visit(BinaryExprAST& node) {
    llvm::Value* leftValue = node.getLHS()->codegen(*this);
    llvm::Value* rightValue = node.getRHS()->codegen(*this);
    if (!leftValue || !rightValue) {
        return nullptr;
    }

    const bool hasFloatOperand = leftValue->getType()->isFloatingPointTy() || rightValue->getType()->isFloatingPointTy();

    switch (node.getOp()) {
        case BinaryExprAST::PLUS:
        case BinaryExprAST::MINUS:
        case BinaryExprAST::MULTIPLY:
        case BinaryExprAST::DIVIDE: {
            if (hasFloatOperand) {
                llvm::Type* floatType = (leftValue->getType()->isDoubleTy() || rightValue->getType()->isDoubleTy())
                    ? llvm::Type::getDoubleTy(TheContext)
                    : llvm::Type::getFloatTy(TheContext);
                leftValue = castValueToType(leftValue, floatType);
                rightValue = castValueToType(rightValue, floatType);
                if (!leftValue || !rightValue) {
                    return logError("Type conversion failed for floating-point arithmetic");
                }

                switch (node.getOp()) {
                    case BinaryExprAST::PLUS:
                        return Builder.CreateFAdd(leftValue, rightValue, "faddtmp");
                    case BinaryExprAST::MINUS:
                        return Builder.CreateFSub(leftValue, rightValue, "fsubtmp");
                    case BinaryExprAST::MULTIPLY:
                        return Builder.CreateFMul(leftValue, rightValue, "fmultmp");
                    case BinaryExprAST::DIVIDE:
                        return Builder.CreateFDiv(leftValue, rightValue, "fdivtmp");
                    default:
                        break;
                }
            }

            if (!leftValue->getType()->isIntegerTy() || !rightValue->getType()->isIntegerTy()) {
                return logError("Arithmetic operators require numeric operands");
            }

            unsigned maxBits = std::max(leftValue->getType()->getIntegerBitWidth(), rightValue->getType()->getIntegerBitWidth());
            if (ExpectedIntegerResultBits.has_value()) {
                maxBits = std::max(maxBits, *ExpectedIntegerResultBits);
            }
            llvm::Type* targetType = llvm::Type::getIntNTy(TheContext, maxBits);
            const bool useUnsignedIntegerOps = isUnsignedValue(leftValue) || isUnsignedValue(rightValue);
            leftValue = castValueToType(leftValue, targetType);
            rightValue = castValueToType(rightValue, targetType);

            switch (node.getOp()) {
                case BinaryExprAST::PLUS:
                    {
                        llvm::Value* result = Builder.CreateAdd(leftValue, rightValue, "addtmp");
                        setValuePrimitiveKind(result, getIntegerPrimitiveKind(maxBits, useUnsignedIntegerOps));
                        return result;
                    }
                case BinaryExprAST::MINUS:
                    {
                        llvm::Value* result = Builder.CreateSub(leftValue, rightValue, "subtmp");
                        setValuePrimitiveKind(result, getIntegerPrimitiveKind(maxBits, useUnsignedIntegerOps));
                        return result;
                    }
                case BinaryExprAST::MULTIPLY:
                    {
                        llvm::Value* result = Builder.CreateMul(leftValue, rightValue, "multmp");
                        setValuePrimitiveKind(result, getIntegerPrimitiveKind(maxBits, useUnsignedIntegerOps));
                        return result;
                    }
                case BinaryExprAST::DIVIDE:
                    {
                        llvm::Value* result = useUnsignedIntegerOps
                            ? Builder.CreateUDiv(leftValue, rightValue, "udivtmp")
                            : Builder.CreateSDiv(leftValue, rightValue, "sdivtmp");
                        setValuePrimitiveKind(result, getIntegerPrimitiveKind(maxBits, useUnsignedIntegerOps));
                        return result;
                    }
                default:
                    break;
            }
            return logError("Invalid arithmetic operator");
        }

        case BinaryExprAST::MODULO: {
            if (!leftValue->getType()->isIntegerTy() || !rightValue->getType()->isIntegerTy()) {
                return logError("Modulo requires integer operands");
            }

            unsigned maxBits = std::max(leftValue->getType()->getIntegerBitWidth(), rightValue->getType()->getIntegerBitWidth());
            if (ExpectedIntegerResultBits.has_value()) {
                maxBits = std::max(maxBits, *ExpectedIntegerResultBits);
            }
            llvm::Type* targetType = llvm::Type::getIntNTy(TheContext, maxBits);
            const bool useUnsignedIntegerOps = isUnsignedValue(leftValue) || isUnsignedValue(rightValue);
            leftValue = castValueToType(leftValue, targetType);
            rightValue = castValueToType(rightValue, targetType);
            llvm::Value* result = useUnsignedIntegerOps
                ? Builder.CreateURem(leftValue, rightValue, "uremtmp")
                : Builder.CreateSRem(leftValue, rightValue, "sremtmp");
            setValuePrimitiveKind(result, getIntegerPrimitiveKind(maxBits, useUnsignedIntegerOps));
            return result;
        }

        case BinaryExprAST::LESS_THAN:
        case BinaryExprAST::LESS_EQUAL:
        case BinaryExprAST::GREATER_THAN:
        case BinaryExprAST::GREATER_EQUAL:
        case BinaryExprAST::EQUAL:
        case BinaryExprAST::NOT_EQUAL: {
            if (hasFloatOperand) {
                llvm::Type* floatType = (leftValue->getType()->isDoubleTy() || rightValue->getType()->isDoubleTy())
                    ? llvm::Type::getDoubleTy(TheContext)
                    : llvm::Type::getFloatTy(TheContext);
                leftValue = castValueToType(leftValue, floatType);
                rightValue = castValueToType(rightValue, floatType);
                if (!leftValue || !rightValue) {
                    return logError("Type conversion failed for floating-point comparison");
                }

                switch (node.getOp()) {
                    case BinaryExprAST::LESS_THAN:
                        return Builder.CreateFCmpOLT(leftValue, rightValue, "fcmp");
                    case BinaryExprAST::LESS_EQUAL:
                        return Builder.CreateFCmpOLE(leftValue, rightValue, "fcmp");
                    case BinaryExprAST::GREATER_THAN:
                        return Builder.CreateFCmpOGT(leftValue, rightValue, "fcmp");
                    case BinaryExprAST::GREATER_EQUAL:
                        return Builder.CreateFCmpOGE(leftValue, rightValue, "fcmp");
                    case BinaryExprAST::EQUAL:
                        return Builder.CreateFCmpOEQ(leftValue, rightValue, "fcmp");
                    case BinaryExprAST::NOT_EQUAL:
                        return Builder.CreateFCmpONE(leftValue, rightValue, "fcmp");
                    default:
                        break;
                }
            }

            if (!leftValue->getType()->isIntegerTy() || !rightValue->getType()->isIntegerTy()) {
                return logError("Comparison requires numeric operands");
            }

            const unsigned maxBits = std::max(leftValue->getType()->getIntegerBitWidth(), rightValue->getType()->getIntegerBitWidth());
            llvm::Type* targetType = llvm::Type::getIntNTy(TheContext, maxBits);
            const bool useUnsignedIntegerOps = isUnsignedValue(leftValue) || isUnsignedValue(rightValue);
            leftValue = castValueToType(leftValue, targetType);
            rightValue = castValueToType(rightValue, targetType);

            switch (node.getOp()) {
                case BinaryExprAST::LESS_THAN:
                    return useUnsignedIntegerOps
                        ? Builder.CreateICmpULT(leftValue, rightValue, "icmp")
                        : Builder.CreateICmpSLT(leftValue, rightValue, "icmp");
                case BinaryExprAST::LESS_EQUAL:
                    return useUnsignedIntegerOps
                        ? Builder.CreateICmpULE(leftValue, rightValue, "icmp")
                        : Builder.CreateICmpSLE(leftValue, rightValue, "icmp");
                case BinaryExprAST::GREATER_THAN:
                    return useUnsignedIntegerOps
                        ? Builder.CreateICmpUGT(leftValue, rightValue, "icmp")
                        : Builder.CreateICmpSGT(leftValue, rightValue, "icmp");
                case BinaryExprAST::GREATER_EQUAL:
                    return useUnsignedIntegerOps
                        ? Builder.CreateICmpUGE(leftValue, rightValue, "icmp")
                        : Builder.CreateICmpSGE(leftValue, rightValue, "icmp");
                case BinaryExprAST::EQUAL:
                    return Builder.CreateICmpEQ(leftValue, rightValue, "icmp");
                case BinaryExprAST::NOT_EQUAL:
                    return Builder.CreateICmpNE(leftValue, rightValue, "icmp");
                default:
                    break;
            }
            return logError("Invalid comparison operator");
        }

        case BinaryExprAST::LOGICAL_AND: {
            llvm::Value* leftBool = castToBoolean(leftValue);
            llvm::Value* rightBool = castToBoolean(rightValue);
            if (!leftBool || !rightBool) {
                return logError("Logical AND requires boolean-compatible operands");
            }
            return Builder.CreateAnd(leftBool, rightBool, "andtmp");
        }

        case BinaryExprAST::LOGICAL_OR: {
            llvm::Value* leftBool = castToBoolean(leftValue);
            llvm::Value* rightBool = castToBoolean(rightValue);
            if (!leftBool || !rightBool) {
                return logError("Logical OR requires boolean-compatible operands");
            }
            return Builder.CreateOr(leftBool, rightBool, "ortmp");
        }

        default:
            return logError("Invalid binary operator");
    }
}

llvm::Value* CodeGenerator::visit(UnaryExprAST& node) {
    llvm::Value* operandValue = node.getOperand()->codegen(*this);
    if (!operandValue) {
        return nullptr;
    }

    switch (node.getOp()) {
        case UnaryExprAST::NEGATE:
            if (operandValue->getType()->isFloatingPointTy()) {
                return Builder.CreateFNeg(operandValue, "fnegtmp");
            }
            if (operandValue->getType()->isIntegerTy()) {
                return Builder.CreateNeg(operandValue, "negtmp");
            }
            return logError("Unary negation requires numeric operand");

        case UnaryExprAST::LOGICAL_NOT: {
            llvm::Value* boolValue = castToBoolean(operandValue);
            if (!boolValue) {
                return logError("Logical NOT requires boolean-compatible operand");
            }
            return Builder.CreateNot(boolValue, "nottmp");
        }

        default:
            return logError("Invalid unary operator");
    }
}

llvm::Value* CodeGenerator::visit(VarDeclStmtAST& node) {
    if (!CurrentFunction) {
        return logError("Variable declaration outside of function");
    }

    llvm::Type* variableType = getLLVMType(node.getType());
    if (!variableType || variableType->isVoidTy()) {
        return logError("Invalid variable type");
    }

    llvm::AllocaInst* variableAlloca = createEntryBlockAlloca(CurrentFunction, variableType, node.getName());
    const PrimitiveTypeAST* primitiveType = getPrimitiveType(node.getType());

    llvm::Value* initValue = nullptr;
    if (node.getInitializer()) {
        const std::optional<unsigned> previousExpectedBits = ExpectedIntegerResultBits;
        if (variableType->isIntegerTy() && !variableType->isIntegerTy(1)) {
            ExpectedIntegerResultBits = variableType->getIntegerBitWidth();
        } else {
            ExpectedIntegerResultBits = std::nullopt;
        }

        initValue = node.getInitializer()->codegen(*this);

        ExpectedIntegerResultBits = previousExpectedBits;
        if (!initValue) {
            return nullptr;
        }
        initValue = castValueToType(initValue, variableType);
        if (!initValue) {
            return logError("Cannot cast initializer to variable type");
        }
    } else {
        initValue = llvm::Constant::getNullValue(variableType);
    }

    Builder.CreateStore(initValue, variableAlloca);
    setNamedValue(node.getName(), variableAlloca);
    if (primitiveType) {
        setNamedPrimitiveKind(node.getName(), primitiveType->getKind());
    }
    return initValue;
}

llvm::Value* CodeGenerator::visit(AssignmentStmtAST& node) {
    llvm::AllocaInst* variableAlloca = getNamedValue(node.getName());
    if (!variableAlloca) {
        return logError("Assignment to unknown variable");
    }

    const std::optional<unsigned> previousExpectedBits = ExpectedIntegerResultBits;
    if (variableAlloca->getAllocatedType()->isIntegerTy() && !variableAlloca->getAllocatedType()->isIntegerTy(1)) {
        ExpectedIntegerResultBits = variableAlloca->getAllocatedType()->getIntegerBitWidth();
    } else {
        ExpectedIntegerResultBits = std::nullopt;
    }

    llvm::Value* assignedValue = node.getValue()->codegen(*this);

    ExpectedIntegerResultBits = previousExpectedBits;
    if (!assignedValue) {
        return nullptr;
    }

    assignedValue = castValueToType(assignedValue, variableAlloca->getAllocatedType());
    if (!assignedValue) {
        return logError("Cannot cast assigned value to variable type");
    }

    Builder.CreateStore(assignedValue, variableAlloca);
    return assignedValue;
}

llvm::Value* CodeGenerator::visit(BlockStmtAST& node) {
    llvm::Value* lastValue = nullptr;
    for (const auto& statement : node.getStatements()) {
        lastValue = statement->codegen(*this);
        if (!lastValue && !Builder.GetInsertBlock()->getTerminator()) {
            return nullptr;
        }
        if (Builder.GetInsertBlock()->getTerminator()) {
            break;
        }
    }

    return lastValue;
}

llvm::Value* CodeGenerator::visit(PrintStmtAST& node) {
    llvm::Value* value = node.getValue()->codegen(*this);
    if (!value) {
        return nullptr;
    }

    llvm::FunctionCallee printfFunction = getPrintfFunction();
    llvm::Type* type = value->getType();
    PrimitiveTypeAST::PrimitiveKind primitiveKind;
    const bool hasPrimitiveKind = getValuePrimitiveKind(value, primitiveKind);

    if (type->isIntegerTy(32)) {
        const bool isUnsigned = hasPrimitiveKind && primitiveKind == PrimitiveTypeAST::UINT32;
        llvm::Value* format = Builder.CreateGlobalString(isUnsigned ? "%u\n" : "%d\n");
        return Builder.CreateCall(printfFunction, {format, value}, "printi32");
    }

    if (type->isDoubleTy()) {
        llvm::Value* format = Builder.CreateGlobalString("%f\n");
        return Builder.CreateCall(printfFunction, {format, value}, "printf64");
    }

    if (type->isFloatTy()) {
        llvm::Value* format = Builder.CreateGlobalString("%f\n");
        llvm::Value* widened = Builder.CreateFPExt(value, llvm::Type::getDoubleTy(TheContext), "f32todouble");
        return Builder.CreateCall(printfFunction, {format, widened}, "printf32");
    }

    if (type->isIntegerTy(1)) {
        llvm::Value* trueString = Builder.CreateGlobalString("true");
        llvm::Value* falseString = Builder.CreateGlobalString("false");
        llvm::Value* selected = Builder.CreateSelect(value, trueString, falseString, "boolstr");
        llvm::Value* format = Builder.CreateGlobalString("%s\n");
        return Builder.CreateCall(printfFunction, {format, selected}, "printbool");
    }

    if (type->isIntegerTy(8)) {
        if (hasPrimitiveKind && primitiveKind == PrimitiveTypeAST::CHAR) {
            llvm::Value* format = Builder.CreateGlobalString("%c\n");
            llvm::Value* widened = Builder.CreateSExt(value, llvm::Type::getInt32Ty(TheContext), "char_to_i32");
            return Builder.CreateCall(printfFunction, {format, widened}, "printchar");
        }

        const bool isUnsigned = hasPrimitiveKind && primitiveKind == PrimitiveTypeAST::UINT8;
        llvm::Value* widened = isUnsigned
            ? Builder.CreateZExt(value, llvm::Type::getInt32Ty(TheContext), "u8_to_i32")
            : Builder.CreateSExt(value, llvm::Type::getInt32Ty(TheContext), "i8_to_i32");
        llvm::Value* format = Builder.CreateGlobalString(isUnsigned ? "%u\n" : "%d\n");
        return Builder.CreateCall(printfFunction, {format, widened}, "printi8");
    }

    if (type->isIntegerTy(16)) {
        const bool isUnsigned = hasPrimitiveKind && primitiveKind == PrimitiveTypeAST::UINT16;
        llvm::Value* widened = isUnsigned
            ? Builder.CreateZExt(value, llvm::Type::getInt32Ty(TheContext), "u16_to_i32")
            : Builder.CreateSExt(value, llvm::Type::getInt32Ty(TheContext), "i16_to_i32");
        llvm::Value* format = Builder.CreateGlobalString(isUnsigned ? "%u\n" : "%d\n");
        return Builder.CreateCall(printfFunction, {format, widened}, "printi16");
    }

    if (type->isIntegerTy(64)) {
        const bool isUnsigned = hasPrimitiveKind && primitiveKind == PrimitiveTypeAST::UINT64;
        llvm::Value* format = Builder.CreateGlobalString(isUnsigned ? "%llu\n" : "%lld\n");
        return Builder.CreateCall(printfFunction, {format, value}, "printi64");
    }

    return logError("Defaultlogger.log only supports primitive scalar values");
}

llvm::Value* CodeGenerator::visit(IfStmtAST& node) {
    if (!CurrentFunction) {
        return logError("If statement outside of function");
    }

    llvm::Value* conditionValue = node.getCondition()->codegen(*this);
    if (!conditionValue) {
        return nullptr;
    }

    conditionValue = castToBoolean(conditionValue);
    if (!conditionValue) {
        return logError("If condition is not boolean-compatible");
    }

    llvm::Function* function = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(TheContext, "then", function);
    llvm::BasicBlock* elseBlock = llvm::BasicBlock::Create(TheContext, "else", function);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(TheContext, "ifcont", function);

    if (node.getElseStmt()) {
        Builder.CreateCondBr(conditionValue, thenBlock, elseBlock);
    } else {
        Builder.CreateCondBr(conditionValue, thenBlock, mergeBlock);
    }

    Builder.SetInsertPoint(thenBlock);
    if (!node.getThenStmt()->codegen(*this) && !Builder.GetInsertBlock()->getTerminator()) {
        return nullptr;
    }
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(mergeBlock);
    }

    if (node.getElseStmt()) {
        Builder.SetInsertPoint(elseBlock);
        if (!node.getElseStmt()->codegen(*this) && !Builder.GetInsertBlock()->getTerminator()) {
            return nullptr;
        }
        if (!Builder.GetInsertBlock()->getTerminator()) {
            Builder.CreateBr(mergeBlock);
        }
    }

    Builder.SetInsertPoint(mergeBlock);

    return llvm::ConstantInt::getTrue(TheContext);
}

llvm::Value* CodeGenerator::visit(WhileStmtAST& node) {
    if (!CurrentFunction) {
        return logError("While statement outside of function");
    }

    llvm::Function* function = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(TheContext, "while.cond", function);
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(TheContext, "while.body", function);
    llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(TheContext, "while.end", function);

    Builder.CreateBr(conditionBlock);

    Builder.SetInsertPoint(conditionBlock);
    llvm::Value* conditionValue = node.getCondition()->codegen(*this);
    if (!conditionValue) {
        return nullptr;
    }
    conditionValue = castToBoolean(conditionValue);
    if (!conditionValue) {
        return logError("While condition is not boolean-compatible");
    }
    Builder.CreateCondBr(conditionValue, bodyBlock, exitBlock);

    Builder.SetInsertPoint(bodyBlock);
    if (!node.getBody()->codegen(*this) && !Builder.GetInsertBlock()->getTerminator()) {
        return nullptr;
    }
    if (!Builder.GetInsertBlock()->getTerminator()) {
        Builder.CreateBr(conditionBlock);
    }

    Builder.SetInsertPoint(exitBlock);

    return llvm::ConstantInt::getTrue(TheContext);
}

llvm::Value* CodeGenerator::visit(MethodImplAST& node) {
    llvm::Type* returnType = getLLVMType(node.getReturnType());
    if (!returnType) {
        return logError("Unknown method return type");
    }

    std::vector<llvm::Type*> parameterTypes;
    parameterTypes.reserve(node.getParams().size());
    for (const auto& parameter : node.getParams()) {
        llvm::Type* parameterType = getLLVMType(parameter->getType());
        if (!parameterType || parameterType->isVoidTy()) {
            return logError("Invalid method parameter type");
        }
        parameterTypes.push_back(parameterType);
    }

    std::string functionName;
    if (CurrentClassIsApplication && node.getName() == "main") {
        functionName = "System.Application.main";
    } else if (!CurrentClassName.empty()) {
        functionName = CurrentClassName + "." + node.getName();
    } else {
        functionName = node.getName();
    }

    llvm::FunctionType* functionType = llvm::FunctionType::get(returnType, parameterTypes, false);
    llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, TheModule.get());

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(TheContext, "entry", function);
    Builder.SetInsertPoint(entryBlock);

    std::map<std::string, llvm::AllocaInst*> previousNamedValues = NamedValues;
    llvm::Function* previousFunction = CurrentFunction;
    CurrentFunction = function;
    NamedValues.clear();

    auto argumentIterator = function->arg_begin();
    for (const auto& parameter : node.getParams()) {
        argumentIterator->setName(parameter->getName());
        llvm::AllocaInst* parameterAlloca = createEntryBlockAlloca(function, argumentIterator->getType(), parameter->getName());
        Builder.CreateStore(argumentIterator, parameterAlloca);
        setNamedValue(parameter->getName(), parameterAlloca);
        ++argumentIterator;
    }

    for (const auto& statement : node.getBody()) {
        if (!statement->codegen(*this) && !Builder.GetInsertBlock()->getTerminator()) {
            function->eraseFromParent();
            NamedValues = previousNamedValues;
            CurrentFunction = previousFunction;
            return nullptr;
        }

        if (Builder.GetInsertBlock()->getTerminator()) {
            break;
        }
    }

    if (!Builder.GetInsertBlock()->getTerminator()) {
        if (returnType->isVoidTy()) {
            Builder.CreateRetVoid();
        } else {
            Builder.CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }

    if (llvm::verifyFunction(*function, &llvm::errs())) {
        function->eraseFromParent();
        NamedValues = previousNamedValues;
        CurrentFunction = previousFunction;
        return logError("Function verification failed");
    }

    NamedValues = previousNamedValues;
    CurrentFunction = previousFunction;
    return function;
}

llvm::Value* CodeGenerator::visit(ClassAST& node) {
    llvm::Value* lastMethod = nullptr;

    const std::string previousClassName = CurrentClassName;
    const bool previousClassIsApplication = CurrentClassIsApplication;

    CurrentClassName = node.getName();
    CurrentClassIsApplication = std::find(node.getBlueprintNames().begin(), node.getBlueprintNames().end(), "Application") != node.getBlueprintNames().end();

    for (const auto& method : node.getMethodImpls()) {
        lastMethod = method->codegen(*this);
        if (!lastMethod) {
            CurrentClassName = previousClassName;
            CurrentClassIsApplication = previousClassIsApplication;
            return nullptr;
        }
    }

    CurrentClassName = previousClassName;
    CurrentClassIsApplication = previousClassIsApplication;
    return lastMethod;
}
