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
    std::unique_ptr<BaseAST> lorexp;

    void Dump() const override {
        std::cout << "ExpAST { ";
        lorexp->Dump();
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        str = lorexp->DumpIR();
        return str;
    }
};

// Primary Auxiliary Data
enum PrimaryType {
    Primary_Exp,
    Primary_Number
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
            case Primary_Exp:
                exp->Dump();
                break;
            case Primary_Number:
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
            case Primary_Exp:
                str = exp->DumpIR();
                break;
            case Primary_Number:
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
    Unary_PrimaryExp,
    Unary_UnaryExp
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
            case Unary_PrimaryExp:
                exp->Dump();
                break;
            case Unary_UnaryExp:
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
            case Unary_PrimaryExp:
                str = exp->DumpIR();
                break;
            case Unary_UnaryExp:
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

// MulExp Auxiliary data
enum MulType {
    Mul_UnaryExp,
    Mul_MulExp,
    Mul_DivExp,
    Mul_ModExp
};

// MulExp
class MulExpAST : public BaseAST {
  public:
    MulType type;
    std::unique_ptr<BaseAST> mulexp;
    std::unique_ptr<BaseAST> unaryexp;
    // char mulop;

    void Dump() const override {
        std::cout << "MulExpAST { ";
        switch(type) {
            case Mul_UnaryExp:
                unaryexp->Dump();
                break;
            case Mul_MulExp:
                mulexp->Dump();
                std::cout << " * ";
                unaryexp->Dump();
                break;
            case Mul_DivExp:
                mulexp->Dump();
                std::cout << " / ";
                unaryexp->Dump();
                break;
            case Mul_ModExp:
                mulexp->Dump();
                std::cout << " \% ";
                unaryexp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case Mul_UnaryExp:
                str = unaryexp->DumpIR();
                break;
            case Mul_MulExp:
                str = unaryexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += mulexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = mul \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Mul_DivExp:
                str = unaryexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += mulexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = div \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Mul_ModExp:
                str = unaryexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += mulexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = mod \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};

// AddExp Auxiliary data
enum AddType {
    Add_MulExp,
    Add_AddExp,
    Add_MinusExp
};

// AddExp
class AddExpAST : public BaseAST {
  public:
    AddType type;
    std::unique_ptr<BaseAST> addexp;
    std::unique_ptr<BaseAST> mulexp;
    // char addop;

    void Dump() const override {
        std::cout << "AddExpAST { ";
        switch(type) {
            case Add_MulExp:
                mulexp->Dump();
                break;
            case Add_AddExp:
                addexp->Dump();
                std::cout << " + ";
                mulexp->Dump();
                break;
            case Add_MinusExp:
                addexp->Dump();
                std::cout << " - ";
                mulexp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case Add_MulExp:
                str = mulexp->DumpIR();
                break;
            case Add_AddExp:
                str = mulexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += addexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = add \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Add_MinusExp:
                str = mulexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += addexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = sub \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};

// RelExp Auxiliary data
enum RelType {
    Rel_AddExp,
    Rel_LTExp,
    Rel_GTExp,
    Rel_LEExp,
    Rel_GEExp
};

// RelExp
class RelExpAST : public BaseAST {
  public:
    RelType type;
    std::unique_ptr<BaseAST> relexp;
    std::unique_ptr<BaseAST> addexp;
    // std::string relop;

    void Dump() const override {
        std::cout << "RelExpAST { ";
        switch(type) {
            case Rel_AddExp:
                addexp->Dump();
                break;
            case Rel_LTExp:
                relexp->Dump();
                std::cout << " < ";
                addexp->Dump();
                break;
            case Rel_GTExp:
                relexp->Dump();
                std::cout << " > ";
                addexp->Dump();
                break;
            case Rel_LEExp:
                relexp->Dump();
                std::cout << " <= ";
                addexp->Dump();
                break;
            case Rel_GEExp:
                relexp->Dump();
                std::cout << " >= ";
                addexp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case Rel_AddExp:
                str = addexp->DumpIR();
                break;
            case Rel_LTExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = lt \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Rel_GTExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = gt \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Rel_LEExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = le \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Rel_GEExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = ge \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};

// EqExp Auxiliary data
enum EqType {
    Eq_RelExp,
    Eq_EQExp,
    Eq_NEQExp
};

// EqExp
class EqExpAST : public BaseAST {
  public:
    EqType type;
    std::unique_ptr<BaseAST> relexp;
    std::unique_ptr<BaseAST> eqexp;
    // std::string eqop;

    void Dump() const override {
        std::cout << "EqExpAST { ";
        switch(type) {
            case Eq_RelExp:
                relexp->Dump();
                break;
            case Eq_EQExp:
                eqexp->Dump();
                std::cout << " == ";
                relexp->Dump();
                break;
            case Eq_NEQExp:
                eqexp->Dump();
                std::cout << " != ";
                relexp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case Eq_RelExp:
                str = relexp->DumpIR();
                break;
            case Eq_EQExp:
                str = relexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += eqexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            case Eq_NEQExp:
                str = relexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += eqexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = ne \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};

// LAndExp Auxiliary data
enum LAndType {
    LAnd_EqExp,
    LAnd_ANDExp
};

// LAndExp
class LAndExpAST : public BaseAST {
  public:
    LAndType type;
    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> landexp;

    void Dump() const override {
        std::cout << "LAndExpAST { ";
        switch(type) {
            case LAnd_EqExp:
                eqexp->Dump();
                break;
            case LAnd_ANDExp:
                landexp->Dump();
                std::cout << " && ";
                eqexp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case LAnd_EqExp:
                str = eqexp->DumpIR();
                break;
            case LAnd_ANDExp:
                str = eqexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += landexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = and \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};

// LOrExp Auxiliary data
enum LOrType {
    LOr_LAndExp,
    LOr_ORExp
};

// LOrExp
class LOrExpAST : public BaseAST {
  public:
    LOrType type;
    std::unique_ptr<BaseAST> landexp;
    std::unique_ptr<BaseAST> lorexp;

    void Dump() const override {
        std::cout << "LOrExpAST { ";
        switch(type) {
            case LOr_LAndExp:
                landexp->Dump();
                break;
            case LOr_ORExp:
                lorexp->Dump();
                std::cout << " || ";
                landexp->Dump();
                break;
            default:
                assert(false);
        }
        std::cout << " }";
    }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case LOr_LAndExp:
                str = landexp->DumpIR();
                break;
            case LOr_ORExp:
                str = landexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += lorexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = or \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }
};