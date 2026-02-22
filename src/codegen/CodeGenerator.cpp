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
    : TheContext(), Builder(TheContext), TheModule(nullptr), NamedValues(), CurrentFunction(nullptr), CurrentClassName(), CurrentClassIsApplication(false) {
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

llvm::AllocaInst* CodeGenerator::getNamedValue(const std::string& name) {
    auto it = NamedValues.find(name);
    if (it != NamedValues.end()) {
        return it->second;
    }
    return nullptr;
}

llvm::Type* CodeGenerator::getLLVMType(const TypeAST* typeAST) {
    auto primitiveType = dynamic_cast<const PrimitiveTypeAST*>(typeAST);
    if (!primitiveType) {
        return nullptr;
    }

    switch (primitiveType->getKind()) {
        case PrimitiveTypeAST::INT32:
            return llvm::Type::getInt32Ty(TheContext);
        case PrimitiveTypeAST::FLOAT32:
            return llvm::Type::getFloatTy(TheContext);
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
        if (sourceBits < targetBits) {
            return Builder.CreateSExt(value, targetType, "sexttmp");
        }
        if (sourceBits > targetBits) {
            return Builder.CreateTrunc(value, targetType, "trunctmp");
        }
        return value;
    }

    if (sourceType->isIntegerTy() && targetType->isFloatingPointTy()) {
        return Builder.CreateSIToFP(value, targetType, "sitofptmp");
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
    return llvm::ConstantInt::get(TheContext, llvm::APInt(32, static_cast<int64_t>(node.getValue()), true));
}

llvm::Value* CodeGenerator::visit(FloatExprAST& node) {
    return llvm::ConstantFP::get(TheContext, llvm::APFloat(static_cast<float>(node.getValue())));
}

llvm::Value* CodeGenerator::visit(BoolExprAST& node) {
    return llvm::ConstantInt::get(TheContext, llvm::APInt(1, node.getValue() ? 1 : 0, false));
}

llvm::Value* CodeGenerator::visit(CharExprAST& node) {
    return llvm::ConstantInt::get(TheContext, llvm::APInt(8, static_cast<uint8_t>(node.getValue()), false));
}

llvm::Value* CodeGenerator::visit(IdentifierExprAST& node) {
    llvm::AllocaInst* alloca = getNamedValue(node.getName());
    if (!alloca) {
        return logError("Unknown variable name");
    }

    return Builder.CreateLoad(alloca->getAllocatedType(), alloca, node.getName() + ".val");
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
                llvm::Type* floatType = llvm::Type::getFloatTy(TheContext);
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

            const unsigned maxBits = std::max(leftValue->getType()->getIntegerBitWidth(), rightValue->getType()->getIntegerBitWidth());
            llvm::Type* targetType = llvm::Type::getIntNTy(TheContext, maxBits);
            leftValue = castValueToType(leftValue, targetType);
            rightValue = castValueToType(rightValue, targetType);

            switch (node.getOp()) {
                case BinaryExprAST::PLUS:
                    return Builder.CreateAdd(leftValue, rightValue, "addtmp");
                case BinaryExprAST::MINUS:
                    return Builder.CreateSub(leftValue, rightValue, "subtmp");
                case BinaryExprAST::MULTIPLY:
                    return Builder.CreateMul(leftValue, rightValue, "multmp");
                case BinaryExprAST::DIVIDE:
                    return Builder.CreateSDiv(leftValue, rightValue, "divtmp");
                default:
                    break;
            }
            return logError("Invalid arithmetic operator");
        }

        case BinaryExprAST::MODULO: {
            if (!leftValue->getType()->isIntegerTy() || !rightValue->getType()->isIntegerTy()) {
                return logError("Modulo requires integer operands");
            }

            const unsigned maxBits = std::max(leftValue->getType()->getIntegerBitWidth(), rightValue->getType()->getIntegerBitWidth());
            llvm::Type* targetType = llvm::Type::getIntNTy(TheContext, maxBits);
            leftValue = castValueToType(leftValue, targetType);
            rightValue = castValueToType(rightValue, targetType);
            return Builder.CreateSRem(leftValue, rightValue, "modtmp");
        }

        case BinaryExprAST::LESS_THAN:
        case BinaryExprAST::LESS_EQUAL:
        case BinaryExprAST::GREATER_THAN:
        case BinaryExprAST::GREATER_EQUAL:
        case BinaryExprAST::EQUAL:
        case BinaryExprAST::NOT_EQUAL: {
            if (hasFloatOperand) {
                llvm::Type* floatType = llvm::Type::getFloatTy(TheContext);
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
            leftValue = castValueToType(leftValue, targetType);
            rightValue = castValueToType(rightValue, targetType);

            switch (node.getOp()) {
                case BinaryExprAST::LESS_THAN:
                    return Builder.CreateICmpSLT(leftValue, rightValue, "icmp");
                case BinaryExprAST::LESS_EQUAL:
                    return Builder.CreateICmpSLE(leftValue, rightValue, "icmp");
                case BinaryExprAST::GREATER_THAN:
                    return Builder.CreateICmpSGT(leftValue, rightValue, "icmp");
                case BinaryExprAST::GREATER_EQUAL:
                    return Builder.CreateICmpSGE(leftValue, rightValue, "icmp");
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

    llvm::Value* initValue = nullptr;
    if (node.getInitializer()) {
        initValue = node.getInitializer()->codegen(*this);
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
    return initValue;
}

llvm::Value* CodeGenerator::visit(AssignmentStmtAST& node) {
    llvm::AllocaInst* variableAlloca = getNamedValue(node.getName());
    if (!variableAlloca) {
        return logError("Assignment to unknown variable");
    }

    llvm::Value* assignedValue = node.getValue()->codegen(*this);
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

    if (type->isIntegerTy(32)) {
        llvm::Value* format = Builder.CreateGlobalString("%d\n");
        return Builder.CreateCall(printfFunction, {format, value}, "printi32");
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
        llvm::Value* format = Builder.CreateGlobalString("%c\n");
        llvm::Value* widened = Builder.CreateSExt(value, llvm::Type::getInt32Ty(TheContext), "char_to_i32");
        return Builder.CreateCall(printfFunction, {format, widened}, "printchar");
    }

    return logError("Defaultlogger.log only supports i32, f32, bool, and char");
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
