#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cassert>

// Base of all AST
class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
    virtual std::string DumpIR() const = 0;
};

static int ast_i = 0;

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
    std::unique_ptr<BaseAST> exp;

    void Dump() const override {
        std::cout << "StmtAST { ";
        exp->Dump();
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        str += exp->DumpIR();
        str += "\tret ";
        str += "\%";
        str += std::to_string(ast_i - 1);
        str += "\n";
        return str;
    }
};

// Exp
class ExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override {
        std::cout << "ExpAST { ";
        unaryexp->Dump();
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        str = unaryexp->DumpIR();
        return str;
    }
};

// Primary Auxiliary Data
enum PrimaryType {
    Exp,
    Number
};

// PrimaryExp
class PrimaryExpAST : public BaseAST {
  public:
    PrimaryType type;
    std::unique_ptr<BaseAST> exp;
    int number;

    void Dump() const override {
        std::cout << "PrimaryExpAST { ";
        switch(type) {
            case Exp:
                exp->Dump();
                break;
            case Number:
                std::cout << number;
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        switch(type) {
            case Exp:
                str = exp->DumpIR();
                break;
            case Number:
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = add ";
                str += std::to_string(number);
                str += ", 0\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};

// Unary Auxiliary Data
enum UnaryType {
    PrimaryExp,
    UnaryExp
};

// UnaryExp
class UnaryExpAST : public BaseAST {
  public:
    UnaryType type;
    std::unique_ptr<BaseAST> exp;
    char unaryop;

    void Dump() const override {
        std::cout << "UnaryExpAST { ";
        switch(type) {
            case PrimaryExp:
                exp->Dump();
                break;
            case UnaryExp:
                std::cout << unaryop << " ";
                exp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        switch(type) {
            case PrimaryExp:
                str = exp->DumpIR();
                break;
            case UnaryExp:
                str = exp->DumpIR();
                if(unaryop == '-'){
                    str += "\t\%";
                    str += std::to_string(ast_i);
                    str += " = sub 0, \%";
                    str += std::to_string(ast_i - 1);
                    str += "\n";
                    ast_i++;
                }
                else if(unaryop == '!'){
                    str += "\t\%";
                    str += std::to_string(ast_i);
                    str += " = eq \%";
                    str += std::to_string(ast_i - 1);
                    str += ", 0\n";
                    ast_i++;
                }
                break;
            default:
                assert(false);
        }
        return str;
    }
};