#pragma once

#include <vector>
#include <memory>
#include <string>

class ExprAST;
class TypeAST;
class PatternAST;
class CaseStmtAST;
class CatchStmtAST;
class MatchCaseAST;

class StmtAST {
    public:
        virtual ~StmtAST() = default;
};

class VarDeclStmtAST : public StmtAST {
    public:
        VarDeclStmtAST(std::unique_ptr<TypeAST> type, const std::string &name, std::unique_ptr<ExprAST> initializer)
            : type(std::move(type)), name(name), initializer(std::move(initializer)) {}
        
        const TypeAST *getType() const { return type.get(); }
        const std::string &getName() const { return name; }
        ExprAST *getInitializer() const { return initializer.get(); }
        
    private:
        std::unique_ptr<TypeAST> type;
        std::string name;
        std::unique_ptr<ExprAST> initializer;
};

class AssignmentStmtAST : public StmtAST {
    public:
        AssignmentStmtAST(const std::string &name, std::unique_ptr<ExprAST> value)
            : name(name), value(std::move(value)) {}
        
        const std::string &getName() const { return name; }
        ExprAST *getValue() const { return value.get(); }
        
    private:
        std::string name;
        std::unique_ptr<ExprAST> value;
};

class IfStmtAST : public StmtAST {
    public:
        IfStmtAST(std::unique_ptr<ExprAST> condition, 
                  std::unique_ptr<StmtAST> thenStmt,
                  std::unique_ptr<StmtAST> elseStmt = nullptr)
            : condition(std::move(condition)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}
        
        ExprAST *getCondition() const { return condition.get(); }
        StmtAST *getThenStmt() const { return thenStmt.get(); }
        StmtAST *getElseStmt() const { return elseStmt.get(); }
        
    private:
        std::unique_ptr<ExprAST> condition;
        std::unique_ptr<StmtAST> thenStmt;
        std::unique_ptr<StmtAST> elseStmt;
};

class WhileStmtAST : public StmtAST {
    public:
        WhileStmtAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<StmtAST> body)
            : condition(std::move(condition)), body(std::move(body)) {}
        
        ExprAST *getCondition() const { return condition.get(); }
        StmtAST *getBody() const { return body.get(); }
        
    private:
        std::unique_ptr<ExprAST> condition;
        std::unique_ptr<StmtAST> body;
};

class BlockStmtAST : public StmtAST {
    public:
        BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> statements)
            : statements(std::move(statements)) {}
        
        const std::vector<std::unique_ptr<StmtAST>> &getStatements() const { return statements; }
        
    private:
        std::vector<std::unique_ptr<StmtAST>> statements;
};

