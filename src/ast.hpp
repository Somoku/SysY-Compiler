#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <unordered_map>

static int ast_i = 0;
static std::unordered_map<std::string, int> symbol_table;

// Base of all AST
class BaseAST {
  public:
    virtual ~BaseAST() = default;
    // virtual void Dump() const = 0;
    virtual std::string DumpIR() const = 0;
    virtual int ConstCalc() const = 0;
};

// CompUnit
class CompUnitAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_def;

    // void Dump() const override {
    //     std::cout << "CompUnitAST { ";
    //     func_def->Dump();
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        return func_def->DumpIR();
    }

    int ConstCalc() const override {
        return 0;
    }
};

// FuncDef
class FuncDefAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    // void Dump() const override {
    //     std::cout << "FuncDefAST { ";
    //     func_type->Dump();
    //     std::cout << ", " << ident << ", ";
    //     block->Dump();
    //     std::cout << " }";
    // }

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

    int ConstCalc() const override {
        return 0;
    }
};

// FuncType
class FuncTypeAST : public BaseAST {
  public:
    std::string int_type;
    
    // void Dump() const override {
    //     std::cout << "FuncTypeAST { ";
    //     std::cout << type_int;
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str("i32 ");
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// Block Auxiliary data
enum BlockType {
    Block_Items,
    Block_Null
};

// Block
class BlockAST : public BaseAST {
  public:
    BlockType type;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > blockitemvec;

    // void Dump() const override {
    //     std::cout << "BlockAST { ";
    //     stmt->Dump();
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str;
        str += "\%entry:\n";
        // str += blockitems->DumpIR();
        size_t vec_size = (*blockitemvec).size();
        for (int i = vec_size - 1; i >= 0; i--)
            str += (*blockitemvec)[i]->DumpIR();
        str += "\n";
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// Stmt
class StmtAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    // void Dump() const override {
    //     std::cout << "StmtAST { ";
    //     exp->Dump();
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str;
        str += exp->DumpIR();
        str += "\tret ";
        str += "\%";
        str += std::to_string(ast_i - 1);
        str += "\n";
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// Exp
class ExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> lorexp;

    // void Dump() const override {
    //     std::cout << "ExpAST { ";
    //     lorexp->Dump();
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str;
        str = lorexp->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return lorexp->ConstCalc();
    }
};

// Primary Auxiliary Data
enum PrimaryType {
    Primary_Exp,
    Primary_Number,
    Primary_Lval
};

// PrimaryExp
class PrimaryExpAST : public BaseAST {
  public:
    PrimaryType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    int number;

    // void Dump() const override {
    //     std::cout << "PrimaryExpAST { ";
    //     switch(type) {
    //         case Primary_Exp:
    //             exp->Dump();
    //             break;
    //         case Primary_Number:
    //             std::cout << number;
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str;
        switch(type) {
            case Primary_Exp:
                str = exp->DumpIR();
                break;
            case Primary_Number:
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = add 0, ";
                str += std::to_string(number);
                str += "\n";
                ast_i++;
                break;
            case Primary_Lval:
                str += lval->DumpIR();
                break;
            default:
                assert(false);
        }
        return str;
    }

    int ConstCalc() const override {
        switch(type) {
            case Primary_Exp:
                return exp->ConstCalc();
                break;
            case Primary_Number:
                return number;
                break;
            case Primary_Lval:
                return lval->ConstCalc();
                break;
            default:
                assert(false);
        }
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

    // void Dump() const override {
    //     std::cout << "UnaryExpAST { ";
    //     switch(type) {
    //         case Unary_PrimaryExp:
    //             exp->Dump();
    //             break;
    //         case Unary_UnaryExp:
    //             std::cout << unaryop << " ";
    //             exp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

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

    int ConstCalc() const override {
        if(type == Unary_PrimaryExp)
            return exp->ConstCalc();
        switch(unaryop) {
            case '+':
                return exp->ConstCalc();
                break;
            case '-':
                return -exp->ConstCalc();
                break;
            case '!':
                return !exp->ConstCalc();
                break;
            default:
                assert(false);
        }
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

    // void Dump() const override {
    //     std::cout << "MulExpAST { ";
    //     switch(type) {
    //         case Mul_UnaryExp:
    //             unaryexp->Dump();
    //             break;
    //         case Mul_MulExp:
    //             mulexp->Dump();
    //             std::cout << " * ";
    //             unaryexp->Dump();
    //             break;
    //         case Mul_DivExp:
    //             mulexp->Dump();
    //             std::cout << " / ";
    //             unaryexp->Dump();
    //             break;
    //         case Mul_ModExp:
    //             mulexp->Dump();
    //             std::cout << " \% ";
    //             unaryexp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

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

    int ConstCalc() const override {
        switch(type) {
            case Mul_UnaryExp:
                return unaryexp->ConstCalc();
                break;
            case Mul_MulExp:
                return (mulexp->ConstCalc() * unaryexp->ConstCalc());
                break;
            case Mul_DivExp:
                return (mulexp->ConstCalc() / unaryexp->ConstCalc());
                break;
            case Mul_ModExp:
                return (mulexp->ConstCalc() % unaryexp->ConstCalc());
                break;
            default:
                assert(false);
        }
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

    // void Dump() const override {
    //     std::cout << "AddExpAST { ";
    //     switch(type) {
    //         case Add_MulExp:
    //             mulexp->Dump();
    //             break;
    //         case Add_AddExp:
    //             addexp->Dump();
    //             std::cout << " + ";
    //             mulexp->Dump();
    //             break;
    //         case Add_MinusExp:
    //             addexp->Dump();
    //             std::cout << " - ";
    //             mulexp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

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

    int ConstCalc() const override {
        switch(type) {
            case Add_MulExp:
                return mulexp->ConstCalc();
                break;
            case Add_MinusExp:
                return (addexp->ConstCalc() - mulexp->ConstCalc());
                break;
            case Add_AddExp:
                return (addexp->ConstCalc() + mulexp->ConstCalc());
                break;
            default:
                assert(false);
        }
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

    // void Dump() const override {
    //     std::cout << "RelExpAST { ";
    //     switch(type) {
    //         case Rel_AddExp:
    //             addexp->Dump();
    //             break;
    //         case Rel_LTExp:
    //             relexp->Dump();
    //             std::cout << " < ";
    //             addexp->Dump();
    //             break;
    //         case Rel_GTExp:
    //             relexp->Dump();
    //             std::cout << " > ";
    //             addexp->Dump();
    //             break;
    //         case Rel_LEExp:
    //             relexp->Dump();
    //             std::cout << " <= ";
    //             addexp->Dump();
    //             break;
    //         case Rel_GEExp:
    //             relexp->Dump();
    //             std::cout << " >= ";
    //             addexp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

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

    int ConstCalc() const override {
        switch(type) {
            case Rel_AddExp:
                return addexp->ConstCalc();
                break;
            case Rel_GEExp:
                return (relexp->ConstCalc() >= addexp->ConstCalc());
                break;
            case Rel_GTExp:
                return (relexp->ConstCalc() > addexp->ConstCalc());
                break;
            case Rel_LEExp:
                return (relexp->ConstCalc() <= addexp->ConstCalc());
                break;
            case Rel_LTExp:
                return (relexp->ConstCalc() < addexp->ConstCalc());
                break;
            default:
                assert(false);
        }
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

    // void Dump() const override {
    //     std::cout << "EqExpAST { ";
    //     switch(type) {
    //         case Eq_RelExp:
    //             relexp->Dump();
    //             break;
    //         case Eq_EQExp:
    //             eqexp->Dump();
    //             std::cout << " == ";
    //             relexp->Dump();
    //             break;
    //         case Eq_NEQExp:
    //             eqexp->Dump();
    //             std::cout << " != ";
    //             relexp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

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

    int ConstCalc() const override {
        switch(type) {
            case Eq_EQExp:
                return (eqexp->ConstCalc() == relexp->ConstCalc());
                break;
            case Eq_NEQExp:
                return (eqexp->ConstCalc() != relexp->ConstCalc());
                break;
            case Eq_RelExp:
                return relexp->ConstCalc();
                break;
            default:
                assert(false);
        }
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

    // void Dump() const override {
    //     std::cout << "LAndExpAST { ";
    //     switch(type) {
    //         case LAnd_EqExp:
    //             eqexp->Dump();
    //             break;
    //         case LAnd_ANDExp:
    //             landexp->Dump();
    //             std::cout << " && ";
    //             eqexp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case LAnd_EqExp:
                str = eqexp->DumpIR();
                break;
            case LAnd_ANDExp:
                str = eqexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                r_num = std::to_string(ast_i - 1);
                str += landexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = or \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }

    int ConstCalc() const override {
        if(type == LAnd_EqExp)
            return eqexp->ConstCalc();
        return landexp->ConstCalc() & eqexp->ConstCalc();
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

    // void Dump() const override {
    //     std::cout << "LOrExpAST { ";
    //     switch(type) {
    //         case LOr_LAndExp:
    //             landexp->Dump();
    //             break;
    //         case LOr_ORExp:
    //             lorexp->Dump();
    //             std::cout << " || ";
    //             landexp->Dump();
    //             break;
    //         default:
    //             assert(false);
    //     }
    //     std::cout << " }";
    // }

    std::string DumpIR() const override {
        std::string str;
        std::string r_num;
        switch(type) {
            case LOr_LAndExp:
                str = landexp->DumpIR();
                break;
            case LOr_ORExp:
                str = landexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                r_num = std::to_string(ast_i - 1);
                str += lorexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = and \%";
                str += std::to_string(ast_i - 1);
                str += ", \%";
                str += r_num;
                str += "\n";
                ast_i++;
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = eq \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                break;
            default:
                assert(false);
        }
        return str;
    }

    int ConstCalc() const override {
        if(type == LOr_LAndExp)
            return landexp->ConstCalc();
        return lorexp->ConstCalc() | landexp->ConstCalc();
    }
};

// Decl
class DeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> constdeclexp;

    std::string DumpIR() const override {
        std::string str;
        str += constdeclexp->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// ConstDecl
class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > constdefvec;

    std::string DumpIR() const override {
        std::string str;
        size_t vec_size = (*constdefvec).size();
        for (int i = vec_size - 1; i >= 0; i--)
            str += (*constdefvec)[i]->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// BType
class BTypeAST : public BaseAST {
  public:
    std::string int_type;

    std::string DumpIR() const override {
        std::string str;
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// ConstDef
class ConstDefAST : public BaseAST {
  public:
    std::string ident;
    std::unique_ptr<BaseAST> constinitval;

    std::string DumpIR() const override {
        std::string str;
        int const_val = ConstCalc();
        symbol_table[ident] = const_val;
        return str;
    }

    int ConstCalc() const override {
        return constinitval->ConstCalc();
    }
};

// ConstInitVal
class ConstInitValAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> constexp;

    std::string DumpIR() const override {
        std::string str;
        return str;
    }

    int ConstCalc() const override {
        return constexp->ConstCalc();
    }
};

// BlockItem Auxiliary data
enum BlockItemType {
    BlockItem_Decl,
    BlockItem_Stmt
};

// BlockItem
class BlockItemAST : public BaseAST {
  public:
    BlockItemType type;
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;

    std::string DumpIR() const override {
        std::string str;
        switch(type) {
            case BlockItem_Decl:
                str += decl->DumpIR();
                break;
            case BlockItem_Stmt:
                str += stmt->DumpIR();
                break;
            default:
                assert(false);
        }
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }
};

// LVal
class LValAST : public BaseAST {
  public:
    std::string ident;

    std::string DumpIR() const override {
        std::string str;
        str += "\t\%";
        str += std::to_string(ast_i);
        str += " = add 0, ";
        str += std::to_string(symbol_table[ident]);
        str += "\n";
        ast_i++;
        return str;
    }

    int ConstCalc() const override {
        return symbol_table[ident];
    }
};

// ConstExp
class ConstExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    std::string DumpIR() const override {
        std::string str;
        return str;
    }

    int ConstCalc() const override {
        return exp->ConstCalc();
    }
};