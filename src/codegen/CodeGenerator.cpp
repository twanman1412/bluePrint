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
    : TheContext(), Builder(TheContext), TheModule(nullptr), NamedValues(), NamedPrimitiveKinds(), ValuePrimitiveKinds(), ExpectedIntegerResultBits(std::nullopt), ExpectedFractionResultKind(std::nullopt), CurrentFunction(nullptr), CurrentClassName(), CurrentClassIsApplication(false) {
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

bool CodeGenerator::isFractionalPrimitiveKind(PrimitiveTypeAST::PrimitiveKind kind) const {
    return kind == PrimitiveTypeAST::FRACTIONAL32 || kind == PrimitiveTypeAST::FRACTIONAL64;
}

bool CodeGenerator::isFractionalValue(llvm::Value* value) const {
    PrimitiveTypeAST::PrimitiveKind kind;
    return getValuePrimitiveKind(value, kind) && isFractionalPrimitiveKind(kind);
}

unsigned CodeGenerator::getFractionalComponentBitWidth(PrimitiveTypeAST::PrimitiveKind kind) const {
    switch (kind) {
        case PrimitiveTypeAST::FRACTIONAL32:
            return 16;
        case PrimitiveTypeAST::FRACTIONAL64:
            return 32;
        default:
            return 32;
    }
}

PrimitiveTypeAST::PrimitiveKind CodeGenerator::getFractionalPrimitiveKindForType(llvm::Type* type) const {
    if (!type || !type->isStructTy()) {
        return PrimitiveTypeAST::INT32;
    }

    auto* structType = llvm::dyn_cast<llvm::StructType>(type);
    if (!structType || structType->getNumElements() != 2) {
        return PrimitiveTypeAST::INT32;
    }

    llvm::Type* numeratorType = structType->getElementType(0);
    llvm::Type* denominatorType = structType->getElementType(1);
    if (!numeratorType->isIntegerTy() || !denominatorType->isIntegerTy()) {
        return PrimitiveTypeAST::INT32;
    }

    if (numeratorType->getIntegerBitWidth() == 16 && denominatorType->getIntegerBitWidth() == 16) {
        return PrimitiveTypeAST::FRACTIONAL32;
    }

    if (numeratorType->getIntegerBitWidth() == 32 && denominatorType->getIntegerBitWidth() == 32) {
        return PrimitiveTypeAST::FRACTIONAL64;
    }

    return PrimitiveTypeAST::INT32;
}

llvm::Type* CodeGenerator::getFractionalComponentType(PrimitiveTypeAST::PrimitiveKind kind) {
    return llvm::Type::getIntNTy(TheContext, getFractionalComponentBitWidth(kind));
}

llvm::StructType* CodeGenerator::getFractionalLLVMType(PrimitiveTypeAST::PrimitiveKind kind) {
    llvm::Type* componentType = getFractionalComponentType(kind);
    return llvm::StructType::get(TheContext, {componentType, componentType});
}

llvm::Function* CodeGenerator::getOrCreateGcdFunction(unsigned bitWidth) {
    const std::string name = "__blueprint_gcd_i" + std::to_string(bitWidth);
    if (llvm::Function* existing = TheModule->getFunction(name)) {
        return existing;
    }

    llvm::Type* intTy = llvm::Type::getIntNTy(TheContext, bitWidth);
    llvm::FunctionType* funcTy = llvm::FunctionType::get(intTy, {intTy, intTy}, false);
    llvm::Function* gcdFunc = llvm::Function::Create(
        funcTy, llvm::Function::InternalLinkage, name, TheModule.get());
    gcdFunc->addFnAttr(llvm::Attribute::NoUnwind);

    llvm::Value* argA = gcdFunc->getArg(0);
    llvm::Value* argB = gcdFunc->getArg(1);
    argA->setName("a");
    argB->setName("b");

    llvm::BasicBlock* entryBB  = llvm::BasicBlock::Create(TheContext, "entry",  gcdFunc);
    llvm::BasicBlock* loopBB   = llvm::BasicBlock::Create(TheContext, "loop",   gcdFunc);
    llvm::BasicBlock* bodyBB   = llvm::BasicBlock::Create(TheContext, "body",   gcdFunc);
    llvm::BasicBlock* returnBB = llvm::BasicBlock::Create(TheContext, "return", gcdFunc);

    // entry: take absolute values so negative numerators don't break GCD
    llvm::IRBuilder<> b(entryBB);
    llvm::Value* zero = llvm::ConstantInt::get(intTy, 0, true);
    llvm::Value* absA = b.CreateSelect(b.CreateICmpSLT(argA, zero), b.CreateNeg(argA, "neg.a"), argA, "abs.a");
    llvm::Value* absB = b.CreateSelect(b.CreateICmpSLT(argB, zero), b.CreateNeg(argB, "neg.b"), argB, "abs.b");
    b.CreateBr(loopBB);

    // loop header: phi nodes for (a, b)
    b.SetInsertPoint(loopBB);
    llvm::PHINode* phiA = b.CreatePHI(intTy, 2, "phi.a");
    llvm::PHINode* phiB = b.CreatePHI(intTy, 2, "phi.b");
    phiA->addIncoming(absA, entryBB);
    phiB->addIncoming(absB, entryBB);
    llvm::Value* bIsZero = b.CreateICmpEQ(phiB, zero, "b.is.zero");
    b.CreateCondBr(bIsZero, returnBB, bodyBB);

    // body: a = b, b = a % b  (Euclidean step)
    b.SetInsertPoint(bodyBB);
    llvm::Value* rem = b.CreateSRem(phiA, phiB, "rem");
    phiA->addIncoming(phiB, bodyBB);
    phiB->addIncoming(rem,  bodyBB);
    b.CreateBr(loopBB);

    // return: when b == 0, a holds the GCD; guard against gcd==0 (both inputs were 0)
    b.SetInsertPoint(returnBB);
    llvm::Value* one = llvm::ConstantInt::get(intTy, 1, true);
    llvm::Value* gcdIsZero = b.CreateICmpEQ(phiA, zero, "gcd.is.zero");
    llvm::Value* safeGcd = b.CreateSelect(gcdIsZero, one, phiA, "safe.gcd");
    b.CreateRet(safeGcd);

    return gcdFunc;
}

llvm::Value* CodeGenerator::buildFractionValue(llvm::Value* numerator, llvm::Value* denominator, PrimitiveTypeAST::PrimitiveKind kind) {
    if (!numerator || !denominator || !isFractionalPrimitiveKind(kind)) {
        return nullptr;
    }

    llvm::Type* componentType = getFractionalComponentType(kind);
    numerator = castValueToType(numerator, componentType);
    denominator = castValueToType(denominator, componentType);
    if (!numerator || !denominator) {
        return nullptr;
    }

    const unsigned bitWidth = getFractionalComponentBitWidth(kind);
    llvm::Type* intTy = componentType;
    llvm::Value* zero = llvm::ConstantInt::get(intTy, 0, true);
    llvm::Value* one  = llvm::ConstantInt::get(intTy, 1, true);
    llvm::Value* negOne = llvm::ConstantInt::get(intTy, -1, true);

    // Sign-normalise: denominator is always positive.
    // If den < 0 flip both signs; if den == 0 force den = 1 (degenerate guard).
    llvm::Value* denIsNeg  = Builder.CreateICmpSLT(denominator, zero, "den.is.neg");
    llvm::Value* denIsZero = Builder.CreateICmpEQ(denominator, zero, "den.is.zero");
    llvm::Value* signFlip  = Builder.CreateSelect(denIsNeg, negOne, one, "sign.flip");
    numerator   = Builder.CreateMul(numerator,   signFlip, "fr.sign.num");
    denominator = Builder.CreateMul(denominator, signFlip, "fr.sign.den");
    denominator = Builder.CreateSelect(denIsZero, one, denominator, "fr.den.safe");

    // GCD reduction
    llvm::Function* gcdFunc = getOrCreateGcdFunction(bitWidth);
    llvm::Value* gcd = Builder.CreateCall(gcdFunc, {numerator, denominator}, "fr.gcd");
    numerator   = Builder.CreateSDiv(numerator,   gcd, "fr.red.num");
    denominator = Builder.CreateSDiv(denominator, gcd, "fr.red.den");

    llvm::StructType* fractionType = getFractionalLLVMType(kind);
    llvm::Value* result = llvm::UndefValue::get(fractionType);
    result = Builder.CreateInsertValue(result, numerator,   {0}, "fr.set.num");
    result = Builder.CreateInsertValue(result, denominator, {1}, "fr.set.den");
    setValuePrimitiveKind(result, kind);
    return result;
}

bool CodeGenerator::decomposeFractionValue(llvm::Value* fractionValue, PrimitiveTypeAST::PrimitiveKind kind, llvm::Value*& numerator, llvm::Value*& denominator) {
    if (!fractionValue || !isFractionalPrimitiveKind(kind)) {
        return false;
    }

    if (!fractionValue->getType()->isStructTy()) {
        return false;
    }

    numerator = Builder.CreateExtractValue(fractionValue, {0}, "fr.num");
    denominator = Builder.CreateExtractValue(fractionValue, {1}, "fr.den");
    return numerator && denominator;
}

llvm::Value* CodeGenerator::castIntegerToFraction(llvm::Value* value, PrimitiveTypeAST::PrimitiveKind targetKind) {
    if (!value || !value->getType()->isIntegerTy() || !isFractionalPrimitiveKind(targetKind)) {
        return nullptr;
    }

    llvm::Type* componentType = getFractionalComponentType(targetKind);
    llvm::Value* numerator = castValueToType(value, componentType);
    if (!numerator) {
        return nullptr;
    }

    llvm::Value* denominator = llvm::ConstantInt::get(componentType, 1, true);
    return buildFractionValue(numerator, denominator, targetKind);
}

llvm::Value* CodeGenerator::castFractionToFloatingPoint(llvm::Value* value, llvm::Type* targetType) {
    if (!value || !targetType || !targetType->isFloatingPointTy()) {
        return nullptr;
    }

    PrimitiveTypeAST::PrimitiveKind kind;
    if (!getValuePrimitiveKind(value, kind) || !isFractionalPrimitiveKind(kind)) {
        return nullptr;
    }

    llvm::Value* numerator = nullptr;
    llvm::Value* denominator = nullptr;
    if (!decomposeFractionValue(value, kind, numerator, denominator)) {
        return nullptr;
    }

    llvm::Value* numeratorFloat = castValueToType(numerator, targetType);
    llvm::Value* denominatorFloat = castValueToType(denominator, targetType);
    if (!numeratorFloat || !denominatorFloat) {
        return nullptr;
    }

    return Builder.CreateFDiv(numeratorFloat, denominatorFloat, "fr.tofp");
}

llvm::Value* CodeGenerator::createFractionArithmetic(int op, llvm::Value* leftValue, llvm::Value* rightValue) {
    if (!leftValue || !rightValue) {
        return nullptr;
    }

    PrimitiveTypeAST::PrimitiveKind leftKind;
    PrimitiveTypeAST::PrimitiveKind rightKind;
    const bool leftHasKind = getValuePrimitiveKind(leftValue, leftKind);
    const bool rightHasKind = getValuePrimitiveKind(rightValue, rightKind);

    PrimitiveTypeAST::PrimitiveKind targetKind = PrimitiveTypeAST::FRACTIONAL32;
    if (leftHasKind && leftKind == PrimitiveTypeAST::FRACTIONAL64) {
        targetKind = PrimitiveTypeAST::FRACTIONAL64;
    }
    if (rightHasKind && rightKind == PrimitiveTypeAST::FRACTIONAL64) {
        targetKind = PrimitiveTypeAST::FRACTIONAL64;
    }
    if (ExpectedFractionResultKind.has_value() && ExpectedFractionResultKind.value() == PrimitiveTypeAST::FRACTIONAL64) {
        targetKind = PrimitiveTypeAST::FRACTIONAL64;
    }

    if (!leftHasKind || !isFractionalPrimitiveKind(leftKind)) {
        leftValue = castIntegerToFraction(leftValue, targetKind);
    } else {
        leftValue = castValueToType(leftValue, getFractionalLLVMType(targetKind));
    }

    if (!rightHasKind || !isFractionalPrimitiveKind(rightKind)) {
        rightValue = castIntegerToFraction(rightValue, targetKind);
    } else {
        rightValue = castValueToType(rightValue, getFractionalLLVMType(targetKind));
    }

    if (!leftValue || !rightValue) {
        return nullptr;
    }

    llvm::Value* leftNumerator = nullptr;
    llvm::Value* leftDenominator = nullptr;
    llvm::Value* rightNumerator = nullptr;
    llvm::Value* rightDenominator = nullptr;
    if (!decomposeFractionValue(leftValue, targetKind, leftNumerator, leftDenominator) ||
        !decomposeFractionValue(rightValue, targetKind, rightNumerator, rightDenominator)) {
        return nullptr;
    }

    const unsigned componentBits = getFractionalComponentBitWidth(targetKind);
    llvm::Type* calcType = llvm::Type::getIntNTy(TheContext, componentBits * 2);

    leftNumerator = castValueToType(leftNumerator, calcType);
    leftDenominator = castValueToType(leftDenominator, calcType);
    rightNumerator = castValueToType(rightNumerator, calcType);
    rightDenominator = castValueToType(rightDenominator, calcType);
    if (!leftNumerator || !leftDenominator || !rightNumerator || !rightDenominator) {
        return nullptr;
    }

    llvm::Value* resultNumerator = nullptr;
    llvm::Value* resultDenominator = nullptr;

    switch (op) {
        case BinaryExprAST::PLUS:
            resultNumerator = Builder.CreateAdd(
                Builder.CreateMul(leftNumerator, rightDenominator, "fr.ln_rd"),
                Builder.CreateMul(rightNumerator, leftDenominator, "fr.rn_ld"),
                "fr.add.num"
            );
            resultDenominator = Builder.CreateMul(leftDenominator, rightDenominator, "fr.add.den");
            break;
        case BinaryExprAST::MINUS:
            resultNumerator = Builder.CreateSub(
                Builder.CreateMul(leftNumerator, rightDenominator, "fr.ln_rd"),
                Builder.CreateMul(rightNumerator, leftDenominator, "fr.rn_ld"),
                "fr.sub.num"
            );
            resultDenominator = Builder.CreateMul(leftDenominator, rightDenominator, "fr.sub.den");
            break;
        case BinaryExprAST::MULTIPLY:
            resultNumerator = Builder.CreateMul(leftNumerator, rightNumerator, "fr.mul.num");
            resultDenominator = Builder.CreateMul(leftDenominator, rightDenominator, "fr.mul.den");
            break;
        case BinaryExprAST::DIVIDE:
            resultNumerator = Builder.CreateMul(leftNumerator, rightDenominator, "fr.div.num");
            resultDenominator = Builder.CreateMul(leftDenominator, rightNumerator, "fr.div.den");
            break;
        default:
            return nullptr;
    }

    llvm::Type* componentType = getFractionalComponentType(targetKind);
    resultNumerator = castValueToType(resultNumerator, componentType);
    resultDenominator = castValueToType(resultDenominator, componentType);
    return buildFractionValue(resultNumerator, resultDenominator, targetKind);
}

llvm::Value* CodeGenerator::createFractionComparison(int op, llvm::Value* leftValue, llvm::Value* rightValue) {
    if (!leftValue || !rightValue) {
        return nullptr;
    }

    PrimitiveTypeAST::PrimitiveKind leftKind;
    PrimitiveTypeAST::PrimitiveKind rightKind;
    const bool leftHasKind = getValuePrimitiveKind(leftValue, leftKind);
    const bool rightHasKind = getValuePrimitiveKind(rightValue, rightKind);

    PrimitiveTypeAST::PrimitiveKind targetKind = PrimitiveTypeAST::FRACTIONAL32;
    if (leftHasKind && leftKind == PrimitiveTypeAST::FRACTIONAL64) {
        targetKind = PrimitiveTypeAST::FRACTIONAL64;
    }
    if (rightHasKind && rightKind == PrimitiveTypeAST::FRACTIONAL64) {
        targetKind = PrimitiveTypeAST::FRACTIONAL64;
    }

    if (!leftHasKind || !isFractionalPrimitiveKind(leftKind)) {
        leftValue = castIntegerToFraction(leftValue, targetKind);
    } else {
        leftValue = castValueToType(leftValue, getFractionalLLVMType(targetKind));
    }

    if (!rightHasKind || !isFractionalPrimitiveKind(rightKind)) {
        rightValue = castIntegerToFraction(rightValue, targetKind);
    } else {
        rightValue = castValueToType(rightValue, getFractionalLLVMType(targetKind));
    }

    if (!leftValue || !rightValue) {
        return nullptr;
    }

    llvm::Value* leftNumerator = nullptr;
    llvm::Value* leftDenominator = nullptr;
    llvm::Value* rightNumerator = nullptr;
    llvm::Value* rightDenominator = nullptr;
    if (!decomposeFractionValue(leftValue, targetKind, leftNumerator, leftDenominator) ||
        !decomposeFractionValue(rightValue, targetKind, rightNumerator, rightDenominator)) {
        return nullptr;
    }

    const unsigned componentBits = getFractionalComponentBitWidth(targetKind);
    llvm::Type* calcType = llvm::Type::getIntNTy(TheContext, componentBits * 2);

    leftNumerator = castValueToType(leftNumerator, calcType);
    leftDenominator = castValueToType(leftDenominator, calcType);
    rightNumerator = castValueToType(rightNumerator, calcType);
    rightDenominator = castValueToType(rightDenominator, calcType);

    llvm::Value* leftCross = Builder.CreateMul(leftNumerator, rightDenominator, "fr.cmp.left");
    llvm::Value* rightCross = Builder.CreateMul(rightNumerator, leftDenominator, "fr.cmp.right");

    switch (op) {
        case BinaryExprAST::LESS_THAN:
            return Builder.CreateICmpSLT(leftCross, rightCross, "fr.cmp");
        case BinaryExprAST::LESS_EQUAL:
            return Builder.CreateICmpSLE(leftCross, rightCross, "fr.cmp");
        case BinaryExprAST::GREATER_THAN:
            return Builder.CreateICmpSGT(leftCross, rightCross, "fr.cmp");
        case BinaryExprAST::GREATER_EQUAL:
            return Builder.CreateICmpSGE(leftCross, rightCross, "fr.cmp");
        case BinaryExprAST::EQUAL:
            return Builder.CreateICmpEQ(leftCross, rightCross, "fr.cmp");
        case BinaryExprAST::NOT_EQUAL:
            return Builder.CreateICmpNE(leftCross, rightCross, "fr.cmp");
        default:
            return nullptr;
    }
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
        case PrimitiveTypeAST::FRACTIONAL32:
            return getFractionalLLVMType(PrimitiveTypeAST::FRACTIONAL32);
        case PrimitiveTypeAST::FRACTIONAL64:
            return getFractionalLLVMType(PrimitiveTypeAST::FRACTIONAL64);
        case PrimitiveTypeAST::BOOL:
            return llvm::Type::getInt1Ty(TheContext);
        case PrimitiveTypeAST::CHAR:
            return llvm::Type::getInt8Ty(TheContext);
        case PrimitiveTypeAST::STR:
            return llvm::PointerType::getUnqual(TheContext);
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

    PrimitiveTypeAST::PrimitiveKind sourceKind;
    const bool hasSourceKind = getValuePrimitiveKind(value, sourceKind);

    if (sourceType->isStructTy() && targetType->isStructTy()) {
        const PrimitiveTypeAST::PrimitiveKind targetKind = getFractionalPrimitiveKindForType(targetType);
        if (hasSourceKind && isFractionalPrimitiveKind(sourceKind) && isFractionalPrimitiveKind(targetKind)) {
            llvm::Value* numerator = nullptr;
            llvm::Value* denominator = nullptr;
            if (!decomposeFractionValue(value, sourceKind, numerator, denominator)) {
                return nullptr;
            }
            return buildFractionValue(numerator, denominator, targetKind);
        }
    }

    if (sourceType->isIntegerTy() && targetType->isStructTy()) {
        const PrimitiveTypeAST::PrimitiveKind targetKind = getFractionalPrimitiveKindForType(targetType);
        if (isFractionalPrimitiveKind(targetKind)) {
            return castIntegerToFraction(value, targetKind);
        }
    }

    if (sourceType->isStructTy() && targetType->isFloatingPointTy()) {
        if (hasSourceKind && isFractionalPrimitiveKind(sourceKind)) {
            return castFractionToFloatingPoint(value, targetType);
        }
    }

    if (sourceType->isStructTy() && targetType->isIntegerTy()) {
        if (hasSourceKind && isFractionalPrimitiveKind(sourceKind)) {
            llvm::Value* numerator = nullptr;
            llvm::Value* denominator = nullptr;
            if (!decomposeFractionValue(value, sourceKind, numerator, denominator)) {
                return nullptr;
            }
            llvm::Value* quotient = Builder.CreateSDiv(numerator, denominator, "fr.toint");
            return castValueToType(quotient, targetType);
        }
    }

    if (sourceType->isFloatingPointTy() && targetType->isStructTy()) {
        return nullptr;
    }

    if (sourceType->isIntegerTy() && targetType->isIntegerTy()) {
        const unsigned sourceBits = sourceType->getIntegerBitWidth();
        const unsigned targetBits = targetType->getIntegerBitWidth();
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

    if (isFractionalValue(value)) {
        PrimitiveTypeAST::PrimitiveKind kind;
        if (!getValuePrimitiveKind(value, kind)) {
            return nullptr;
        }

        llvm::Value* numerator = nullptr;
        llvm::Value* denominator = nullptr;
        if (!decomposeFractionValue(value, kind, numerator, denominator)) {
            return nullptr;
        }

        llvm::Value* zero = llvm::ConstantInt::get(numerator->getType(), 0, true);
        return Builder.CreateICmpNE(numerator, zero, "booltmp");
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

llvm::Value* CodeGenerator::visit(StrExprAST& node) {
    llvm::Value* strPtr = Builder.CreateGlobalString(node.getValue(), "str.literal");
    setValuePrimitiveKind(strPtr, PrimitiveTypeAST::STR);
    return strPtr;
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
    const bool hasFractionalOperand = isFractionalValue(leftValue) || isFractionalValue(rightValue);

    switch (node.getOp()) {
        case BinaryExprAST::PLUS:
        case BinaryExprAST::MINUS:
        case BinaryExprAST::MULTIPLY:
        case BinaryExprAST::DIVIDE: {
            const bool expectsFractionResult = ExpectedFractionResultKind.has_value() && node.getOp() == BinaryExprAST::DIVIDE && !hasFloatOperand;
            if ((hasFractionalOperand || expectsFractionResult) && !hasFloatOperand) {
                llvm::Value* result = createFractionArithmetic(node.getOp(), leftValue, rightValue);
                if (!result) {
                    return logError("Type conversion failed for fractional arithmetic");
                }
                return result;
            }

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
            if (hasFractionalOperand && !hasFloatOperand) {
                llvm::Value* result = createFractionComparison(node.getOp(), leftValue, rightValue);
                if (!result) {
                    return logError("Type conversion failed for fractional comparison");
                }
                return result;
            }

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
        const std::optional<PrimitiveTypeAST::PrimitiveKind> previousExpectedFractionKind = ExpectedFractionResultKind;
        if (variableType->isIntegerTy() && !variableType->isIntegerTy(1)) {
            ExpectedIntegerResultBits = variableType->getIntegerBitWidth();
        } else {
            ExpectedIntegerResultBits = std::nullopt;
        }

        if (primitiveType && isFractionalPrimitiveKind(primitiveType->getKind())) {
            ExpectedFractionResultKind = primitiveType->getKind();
        } else {
            ExpectedFractionResultKind = std::nullopt;
        }

        initValue = node.getInitializer()->codegen(*this);

        ExpectedIntegerResultBits = previousExpectedBits;
        ExpectedFractionResultKind = previousExpectedFractionKind;
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
    const std::optional<PrimitiveTypeAST::PrimitiveKind> previousExpectedFractionKind = ExpectedFractionResultKind;
    PrimitiveTypeAST::PrimitiveKind variableKind;
    const bool hasVariableKind = getNamedPrimitiveKind(node.getName(), variableKind);
    if (variableAlloca->getAllocatedType()->isIntegerTy() && !variableAlloca->getAllocatedType()->isIntegerTy(1)) {
        ExpectedIntegerResultBits = variableAlloca->getAllocatedType()->getIntegerBitWidth();
    } else {
        ExpectedIntegerResultBits = std::nullopt;
    }

    if (hasVariableKind && isFractionalPrimitiveKind(variableKind)) {
        ExpectedFractionResultKind = variableKind;
    } else {
        ExpectedFractionResultKind = std::nullopt;
    }

    llvm::Value* assignedValue = node.getValue()->codegen(*this);

    ExpectedIntegerResultBits = previousExpectedBits;
    ExpectedFractionResultKind = previousExpectedFractionKind;
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

    if (hasPrimitiveKind && isFractionalPrimitiveKind(primitiveKind)) {
        llvm::Value* numerator = nullptr;
        llvm::Value* denominator = nullptr;
        if (!decomposeFractionValue(value, primitiveKind, numerator, denominator)) {
            return logError("Could not print fractional value");
        }

        if (primitiveKind == PrimitiveTypeAST::FRACTIONAL32) {
            llvm::Value* format = Builder.CreateGlobalString("%d/%d\n");
            llvm::Value* widenedNum = Builder.CreateSExt(numerator, llvm::Type::getInt32Ty(TheContext), "fr32_num_i32");
            llvm::Value* widenedDen = Builder.CreateSExt(denominator, llvm::Type::getInt32Ty(TheContext), "fr32_den_i32");
            return Builder.CreateCall(printfFunction, {format, widenedNum, widenedDen}, "printfr32");
        }

        llvm::Value* format = Builder.CreateGlobalString("%d/%d\n");
        return Builder.CreateCall(printfFunction, {format, numerator, denominator}, "printfr64");
    }

    if (hasPrimitiveKind && primitiveKind == PrimitiveTypeAST::STR) {
        llvm::Value* format = Builder.CreateGlobalString("%s\n");
        return Builder.CreateCall(printfFunction, {format, value}, "printstr");
    }

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
    std::map<std::string, PrimitiveTypeAST::PrimitiveKind> previousNamedPrimitiveKinds = NamedPrimitiveKinds;
    llvm::Function* previousFunction = CurrentFunction;
    CurrentFunction = function;
    NamedValues.clear();
    NamedPrimitiveKinds.clear();

    auto argumentIterator = function->arg_begin();
    for (const auto& parameter : node.getParams()) {
        argumentIterator->setName(parameter->getName());
        llvm::AllocaInst* parameterAlloca = createEntryBlockAlloca(function, argumentIterator->getType(), parameter->getName());
        Builder.CreateStore(argumentIterator, parameterAlloca);
        setNamedValue(parameter->getName(), parameterAlloca);
        const PrimitiveTypeAST* parameterPrimitiveType = getPrimitiveType(parameter->getType());
        if (parameterPrimitiveType) {
            setNamedPrimitiveKind(parameter->getName(), parameterPrimitiveType->getKind());
        }
        ++argumentIterator;
    }

    for (const auto& statement : node.getBody()) {
        if (!statement->codegen(*this) && !Builder.GetInsertBlock()->getTerminator()) {
            function->eraseFromParent();
            NamedValues = previousNamedValues;
            NamedPrimitiveKinds = previousNamedPrimitiveKinds;
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
        NamedPrimitiveKinds = previousNamedPrimitiveKinds;
        CurrentFunction = previousFunction;
        return logError("Function verification failed");
    }

    NamedValues = previousNamedValues;
    NamedPrimitiveKinds = previousNamedPrimitiveKinds;
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
