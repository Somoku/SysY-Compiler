#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <unordered_map>
#include "symbol.hpp"

static int ast_i = 0;
extern symbol_table_list_elem_t *curr_symbol_table;

// Base of all AST
class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual std::string DumpIR() const = 0;
    virtual int ConstCalc() const = 0;
    virtual std::string getIdent() const = 0;
};

// CompUnit
class CompUnitAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_def;

    std::string DumpIR() const override {
        return func_def->DumpIR();
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// FuncDef
class FuncDefAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    std::string DumpIR() const override {
        std::string str;
        str += "fun @";
        str += ident;
        str += "(): ";
        str += func_type->DumpIR();
        str += "{\n";
        str += "\%entry:\n";
        str += block->DumpIR();
        str += "}\n";
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// FuncType
class FuncTypeAST : public BaseAST {
  public:
    std::string int_type;

    std::string DumpIR() const override {
        std::string str("i32 ");
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
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

    std::string DumpIR() const override {
        std::string str;
        if(type == Block_Items) {
            create_symbol_table();
            size_t vec_size = (*blockitemvec).size();
            for (int i = vec_size - 1; i >= 0; i--)
                str += (*blockitemvec)[i]->DumpIR();
            delete_symbol_table();
        }
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// Stmt Auxiliary data
enum StmtType {
    Stmt_Ret,
    Stmt_Lval,
    Stmt_Exp,
    Stmt_Null,
    Stmt_Block,
    Stmt_Ret_Null
};

// Stmt
class StmtAST : public BaseAST {
  public:
    StmtType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;

    std::string DumpIR() const override {
        std::string str;
        switch(type) {
            case Stmt_Ret:
                str += exp->DumpIR();
                str += "\tret ";
                str += "\%";
                str += std::to_string(ast_i - 1);
                str += "\n";
                break;
            case Stmt_Lval:
                str += exp->DumpIR();
                str += "\tstore \%";
                str += std::to_string(ast_i - 1);
                str += ", @";
                str += lval->getIdent();
                str += "\n";
                break;
            case Stmt_Exp:
                str += exp->DumpIR();
                break;
            case Stmt_Null:
                break;
            case Stmt_Block:
                str += exp->DumpIR();
                break;
            case Stmt_Ret_Null:
                str += "\tret\n";
                break;
            default:
                assert(false);
        }
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// Exp
class ExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> lorexp;

    std::string DumpIR() const override {
        std::string str;
        str = lorexp->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return lorexp->ConstCalc();
    }

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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
        return landexp->ConstCalc() && eqexp->ConstCalc();
    }

    std::string getIdent() const override {
        return std::string();
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
        return lorexp->ConstCalc() || landexp->ConstCalc();
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// Decl Auxiliary data
enum DeclType {
    Decl_Const,
    Decl_Var
};

// Decl
class DeclAST : public BaseAST {
  public:
    DeclType type;
    std::unique_ptr<BaseAST> exp;

    std::string DumpIR() const override {
        std::string str;
        str = exp->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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
        (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
            symbol_t{const_val, symbol_tag::Symbol_Const};
        return str;
    }

    int ConstCalc() const override {
        return constinitval->ConstCalc();
    }

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
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

    std::string getIdent() const override {
        return std::string();
    }
};

// LVal
class LValAST : public BaseAST {
  public:
    std::string ident;

    std::string DumpIR() const override {
        std::string str;
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        if(target_symbol_table == nullptr) {
            std::cerr << "Error: Can't find ident." << std::endl;
            str = ident;
        }
        else {
            symbol_t symbol = (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident];
            int symbol_num = target_symbol_table->symbol_table_ptr->symbol_table_num;
            if(symbol.tag == 
                symbol_tag::Symbol_Const) {
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = add 0, ";
                str += std::to_string(symbol.value);
                str += "\n";
                ast_i++;
            }
            else {
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = load @";
                str += ident;
                str += "_";
                str += std::to_string(symbol_num);
                str += "\n";
                ast_i++;
            }
        }
        return str;
    }

    int ConstCalc() const override {
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        assert(target_symbol_table != nullptr);
        return (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident].value;
    }

    std::string getIdent() const override {
        std::string str;
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        int symbol_num = target_symbol_table->symbol_table_ptr->symbol_table_num;
        str += ident;
        str += "_";
        str += std::to_string(symbol_num);
        return str;
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

    std::string getIdent() const override {
        return std::string();
    }
};

// VarDecl
class VarDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > vardefvec;

    std::string DumpIR() const override {
        std::string str;
        size_t vec_size = (*vardefvec).size();
        for (int i = vec_size - 1; i >= 0; i--)
            str += (*vardefvec)[i]->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// VarDef Auxiliary data
enum VarDefType {
    VarDef_NO_Init,
    VarDef_Init
};

// VarDef
class VarDefAST : public BaseAST {
  public:
    VarDefType type;
    std::string ident;
    std::unique_ptr<BaseAST> initval;

    std::string DumpIR() const override {
        std::string str;
        str += "\t@";
        str += ident;
        str += "_";
        str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
        str += " = alloc i32\n";
        if(type == VarDef_Init) {
            str += initval->DumpIR();
            str += "\tstore \%";
            str += std::to_string(ast_i - 1);
            str += ", @";
            str += ident;
            str += "_";
            str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
            str += "\n";
        }
        (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
            symbol_t{-1, symbol_tag::Symbol_Var};
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};

// InitVal
class InitValAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;

    std::string DumpIR() const override {
        std::string str;
        str += exp->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }
};