#pragma once

#include <memory>
#include <string>
#include <iostream>

// Base of all AST
class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual std::string DumpIR() const = 0;
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

    std::string DumpIR() const override {
        return func_def->DumpIR();
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

    std::string DumpIR() const override {
        std::string str;
        str += "fun @";
        str += ident;
        str += "(): ";
        str += func_type->DumpIR();
        str += "{\n";
        str += block->DumpIR();
        str += "}\n";
        return str;
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

    std::string DumpIR() const override {
        std::string str("i32 ");
        return str;
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

    std::string DumpIR() const override {
        std::string str;
        str += "\%entry:\n";
        str += stmt->DumpIR();
        str += "\n";
        return str;
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

    std::string DumpIR() const override {
        std::string str;
        str += "\tret ";
        str += std::to_string(number);
        str += "\n";
        return str;
    }
};
