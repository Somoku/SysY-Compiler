#pragma once
#include <memory>
#include <string>
#include <iostream>

// Base of all AST
class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual void DumpIR() const = 0;
};

// CompUnit
class CompUnitAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        std::cout << "CompUnitAST { ";
        func_def->Dump();
        std::cout << " }";
    }

    void DumpIR() const override {
        func_def->DumpIR();
    }
};

// FuncDef
class FuncDefAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        std::cout << "FuncDefAST { ";
        func_type->Dump();
        std::cout << ", " << ident << ", ";
        block->Dump();
        std::cout << " }";
    }

    void DumpIR() const override {
        std::cout << "fun @";
        std::cout << ident;
        std::cout << "(): ";
        func_type->DumpIR();
        std::cout << "{" << std::endl;
        block->DumpIR();
        std::cout << "}" << std::endl;
    }
};

// FuncType
class FuncTypeAST : public BaseAST {
  public:
    std::string type_int;
    
    void Dump() const override {
        std::cout << "FuncTypeAST { ";
        std::cout << type_int;
        std::cout << " }";
    }

    void DumpIR() const override {
        std::cout << "i32 ";
    }
};

// Block
class BlockAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "BlockAST { ";
        stmt->Dump();
        std::cout << " }";
    }

    void DumpIR() const override {
        std::cout << "\%entry:" << std::endl;
        stmt->DumpIR();
        std::cout << std::endl;
    }
};

// Stmt
class StmtAST : public BaseAST {
  public:
    int number;

    void Dump() const override {
        std::cout << "StmtAST { ";
        std::cout << number;
        std::cout << " }";
    }

    void DumpIR() const override {
        std::cout << "\tret ";
        std::cout << number;
        std::cout << std::endl;
    }
};
