#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <unordered_map>
#include "symbol.hpp"

static int ast_i = 0;
static int block_id = 0;
static int logical_id = 0;
static int while_id = 0;
static int entry_id = 0;
static bool block_ret = false;
static bool block_jump = false;
static symbol_field field = symbol_field::Field_Global;
static std::vector<int> while_id_vec;
extern symbol_table_list_elem_t *curr_symbol_table;
extern std::unordered_map<std::string, symbol_t> global_symbol_table;

// Base of all AST
class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual std::string DumpIR() const = 0;
    virtual int ConstCalc() const = 0;
    virtual std::string getIdent() const = 0;
    virtual std::string getPointer() const = 0;
    virtual std::unique_ptr<std::vector<int> > getArrInit() const = 0;
};

// CompUnitRoot
class CompUnitRootAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> compunit;

    std::string DumpIR() const override {
        std::string str;
        str += koopa_lib();
        str += compunit->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// CompUnit Auxiliary data
enum CompUnitType {
    CompUnit_SinFunc,
    CompUnit_MulFunc,
    CompUnit_SinDecl,
    CompUnit_MulDecl
};

// CompUnit
class CompUnitAST : public BaseAST {
  public:
    CompUnitType type;
    std::unique_ptr<BaseAST> func_def;
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> compunit;

    std::string DumpIR() const override {
        std::string str;
        switch(type) {
            case CompUnit_SinFunc:
                str += func_def->DumpIR();
                field = symbol_field::Field_Global;
                break;
            case CompUnit_MulFunc:
                field = symbol_field::Field_Global;
                str += compunit->DumpIR();
                field = symbol_field::Field_Global;
                str += func_def->DumpIR();
                field = symbol_field::Field_Global;
                break;
            case CompUnit_SinDecl:
                str += decl->DumpIR();
                field = symbol_field::Field_Global;
                break;
            case CompUnit_MulDecl:
                field = symbol_field::Field_Global;
                str += compunit->DumpIR();
                field = symbol_field::Field_Global;
                str += decl->DumpIR();
                field = symbol_field::Field_Global;
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// FuncDef Auxiliary data
enum FuncDefType {
    FuncDef_Noparam,
    FuncDef_Param
};

// FuncDef
class FuncDefAST : public BaseAST {
  public:
    FuncDefType type;
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > funcfparamvec;
    std::unique_ptr<BaseAST> block;

    std::string DumpIR() const override {
        std::string str;
        // std::string func_ident;
        // symbol_table_list_elem_t *target_symbol_table;
        global_symbol_table[ident] = symbol_t{func_type->ConstCalc(), symbol_tag::Symbol_Func};
        field = symbol_field::Field_Local;
        if(type == FuncDef_Noparam) {
            str += "fun @";
            str += ident;
            str += "()";
            str += func_type->DumpIR();
            str += "{\n";
            str += "\%entry_";
            str += std::to_string(entry_id++);
            str += ":\n";
            block_ret = false;
            block_jump = false;
            str += block->DumpIR();
            if(!block_ret && !block_jump)
                str += "\tret\n";
            str += "}\n";
        }
        else {
            str += "fun @";
            str += ident;
            str += "(";
            create_symbol_table();
            size_t vec_size = funcfparamvec->size();
            for(int i = vec_size - 1; i > 0; i--) {
                str += (*funcfparamvec)[i]->getIdent();
                str += ", ";
            }
            str += (*funcfparamvec)[0]->getIdent();
            str += ")";
            str += func_type->DumpIR();
            str += " {\n";
            str += "\%entry_";
            str += std::to_string(entry_id++);
            str += ":\n";
            block_ret = false;
            block_jump = false;
            for(int i = vec_size - 1; i >= 0; i--)
                str += (*funcfparamvec)[i]->DumpIR();
            str += block->DumpIR();
            if(!block_ret && !block_jump)
                str += "\tret\n";
            str += "}\n";
            delete_symbol_table();
        }
        field = symbol_field::Field_Global;
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// FuncType Auxiliary data
enum FuncTypeType {
    FuncType_INT,
    FuncType_VOID
};

// FuncType
class FuncTypeAST : public BaseAST {
  public:
    FuncTypeType type;

    std::string DumpIR() const override {
        std::string str;
        if(type == FuncType_INT)
            str = ": i32 ";
        else
            str = " ";
        return str;
    }

    int ConstCalc() const override {
        return (int)type;
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// FuncFParam
class FuncFParamAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> btype;
    std::string ident;

    std::string DumpIR() const override {
        std::string str;
        std::string param_ident = std::string("param_") + ident + "_" +
                                std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
        str += "\t@";
        str += ident;
        str += "_";
        str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
        str += " = alloc i32\n";
        str += "\tstore @";
        str += param_ident;
        str += ", @";
        str += ident;
        str += "_";
        str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
        str += "\n";
        (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
            symbol_t{-1, symbol_tag::Symbol_Var};
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        std::string str;
        std::string param_ident = std::string("param_") + ident;
        str += "@param_";
        str += ident;
        str += "_";
        str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
        str += ": i32";
        return str;
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// Stmt Auxiliary data
enum StmtType {
    Stmt_Open,
    Stmt_Closed
};

// Stmt
class StmtAST : public BaseAST {
  public:
    StmtType type;
    std::unique_ptr<BaseAST> stmt;
    
    std::string DumpIR() const override {
        std::string str;
        str += stmt->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// OpenStmt Auxiliary data
enum OpenStmtType {
    Open_If,
    Open_Else,
    Open_While
};

// OpenStmt
class OpenStmtAST : public BaseAST {
  public:
    OpenStmtType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> stmt;
    std::unique_ptr<BaseAST> openstmt;

    std::string DumpIR() const override {
        std::string str;
        int then_id = 0;
        int else_id = 0;
        int end_id = 0;
        int while_block_id = 0;
        bool then_ret = false;
        bool else_ret = false;
        switch(type) {
            case Open_If:
                then_id = block_id++;
                end_id = block_id++;
                str += exp->DumpIR();
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%block_";
                str += std::to_string(then_id);
                str += ", \%block_";
                str += std::to_string(end_id);
                str += "\n";
                str += "\%block_";
                str += std::to_string(then_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += stmt->DumpIR();
                if(!block_ret && !block_jump) {
                    str += "\tjump \%block_";
                    str += std::to_string(end_id);
                    str += "\n";
                }
                str += "\%block_";
                str += std::to_string(end_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                break;
            case Open_Else:
                then_id = block_id++;
                else_id = block_id++;
                end_id = block_id++;
                str += exp->DumpIR();
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%block_";
                str += std::to_string(then_id);
                str += ", \%block_";
                str += std::to_string(else_id);
                str += "\n";
                str += "\%block_";
                str += std::to_string(then_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += stmt->DumpIR();
                then_ret = block_ret;
                if(!block_ret && !block_jump) {
                    str += "\tjump \%block_";
                    str += std::to_string(end_id);
                    str += "\n";
                }
                str += "\%block_";
                str += std::to_string(else_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += openstmt->DumpIR();
                else_ret = block_ret;
                if(!block_ret && !block_jump) {
                    str += "\tjump \%block_";
                    str += std::to_string(end_id);
                    str += "\n";
                }
                if(!then_ret || !else_ret){
                    str += "\%block_";
                    str += std::to_string(end_id);
                    str += ":\n";
                    block_ret = false;
                }
                break;
            case Open_While:
                while_id_vec.push_back(while_id);
                while_block_id = while_id;
                while_id++;
                str += "\tjump \%while_entry_";
                str += std::to_string(while_block_id);
                str += "\n";
                str += "\%while_entry_";
                str += std::to_string(while_block_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += exp->DumpIR();
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%while_body_";
                str += std::to_string(while_block_id);
                str += ", \%while_end_";
                str += std::to_string(while_block_id);
                str += "\n";
                str += "\%while_body_";
                str += std::to_string(while_block_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += openstmt->DumpIR();
                if(!block_ret && !block_jump){
                    str += "\tjump \%while_entry_";
                    str += std::to_string(while_block_id);
                    str += "\n";
                }
                while_id_vec.pop_back();
                str += "\%while_end_";
                str += std::to_string(while_block_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// ClosedStmt Auxiliary data
enum ClosedStmtType {
    Closed_NonIf,
    Closed_If,
    Closed_While
};

// ClosedStmt
class ClosedStmtAST : public BaseAST {
  public:
    ClosedStmtType type;
    std::unique_ptr<BaseAST> stmt;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> closedstmt;

    std::string DumpIR() const override {
        std::string str;
        int then_id = 0;
        int else_id = 0;
        int end_id = 0;
        int while_block_id = 0;
        bool then_ret = false;
        bool else_ret = false;
        switch(type) {
            case Closed_NonIf:
                str += stmt->DumpIR();
                break;
            case Closed_If:
                then_id = block_id++;
                else_id = block_id++;
                end_id = block_id++;
                str += exp->DumpIR();
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%block_";
                str += std::to_string(then_id);
                str += ", \%block_";
                str += std::to_string(else_id);
                str += "\n";
                str += "\%block_";
                str += std::to_string(then_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += stmt->DumpIR();
                then_ret = block_ret;
                if(!block_ret && !block_jump) {
                    str += "\tjump \%block_";
                    str += std::to_string(end_id);
                    str += "\n";   
                }
                str += "\%block_";
                str += std::to_string(else_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += closedstmt->DumpIR();
                else_ret = block_ret;
                if(!block_ret && !block_jump) {
                    str += "\tjump \%block_";
                    str += std::to_string(end_id);
                    str += "\n"; 
                }
                if(!then_ret || !else_ret){
                    str += "\%block_";
                    str += std::to_string(end_id);
                    str += ":\n";
                    block_ret = false;
                    block_jump = false;
                }
                break;
            case Closed_While:
                while_id_vec.push_back(while_id);
                while_block_id = while_id;
                while_id++;
                str += "\tjump \%while_entry_";
                str += std::to_string(while_block_id);
                str += "\n";
                str += "\%while_entry_";
                str += std::to_string(while_block_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += exp->DumpIR();
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%while_body_";
                str += std::to_string(while_block_id);
                str += ", \%while_end_";
                str += std::to_string(while_block_id);
                str += "\n";
                str += "\%while_body_";
                str += std::to_string(while_block_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
                str += stmt->DumpIR();
                if(!block_ret && !block_jump){
                    str += "\tjump \%while_entry_";
                    str += std::to_string(while_block_id);
                    str += "\n";
                }
                while_id_vec.pop_back();
                str += "\%while_end_";
                str += std::to_string(while_block_id);
                str += ":\n";
                block_ret = false;
                block_jump = false;
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// NonIfStmt Auxiliary data
enum NonIfStmtType {
    NonIf_Ret,
    NonIf_Lval,
    NonIf_Exp,
    NonIf_Null,
    NonIf_Block,
    NonIf_Ret_Null,
    NonIf_If,
    NonIf_If_Else,
    NonIf_Break,
    NonIf_Continue
};

// NonIfStmt
class NonIfStmtAST : public BaseAST {
  public:
    NonIfStmtType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;

    std::string DumpIR() const override {
        std::string str;
        int while_block_id = 0;
        std::string ident;
        symbol_table_list_elem_t *target_symbol_table;
        symbol_tag tag;
        int store_src;
        switch(type) {
            case NonIf_Ret:
                str += exp->DumpIR();
                str += "\tret ";
                str += "\%";
                str += std::to_string(ast_i - 1);
                str += "\n";
                block_ret = true;
                break;
            case NonIf_Lval:
                str += exp->DumpIR();
                store_src = ast_i - 1;
                ident = lval->getIdent();
                target_symbol_table = search_symbol_table(ident);   
                if(target_symbol_table != nullptr) {
                    tag = (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident].tag;
                    ident += "_";
                    ident += std::to_string(target_symbol_table->symbol_table_ptr->symbol_table_num);
                }
                else if(global_symbol_table.find(ident) != global_symbol_table.end())
                    tag = global_symbol_table[ident].tag;
                else {
                    std::cerr << "Error: Invalid ident.\n";
                    assert(false);
                }
                switch(tag) {
                    case Symbol_Const:
                    case Symbol_Var:
                        str += "\tstore \%";
                        str += std::to_string(ast_i - 1);
                        str += ", @";
                        str += ident;
                        str += "\n";
                        break;
                    case Symbol_Arr:
                        str += lval->getPointer();
                        str += "\tstore \%";
                        str += std::to_string(store_src);
                        str += ", \%";
                        str += std::to_string(ast_i - 1);
                        str += "\n";
                        break;
                    default:
                        assert(false);
                }
                break;
            case NonIf_Exp:
                str += exp->DumpIR();
                break;
            case NonIf_Null:
                break;
            case NonIf_Block:
                str += exp->DumpIR();
                break;
            case NonIf_Ret_Null:
                str += "\tret\n";
                block_ret = true;
                break;
            case NonIf_Break:
                if(while_id_vec.empty())
                    break;
                while_block_id = while_id_vec[while_id_vec.size() - 1];
                if(!block_ret && !block_jump){
                    str += "\tjump \%while_end_";
                    str += std::to_string(while_block_id);
                    str += "\n";
                    block_jump = true;
                }
                break;
            case NonIf_Continue:
                if(while_id_vec.empty())
                    break;
                while_block_id = while_id_vec[while_id_vec.size() - 1];
                if(!block_ret && !block_jump){
                    str += "\tjump \%while_entry_";
                    str += std::to_string(while_block_id);
                    str += "\n";
                    block_jump = true;
                }
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// Unary Auxiliary Data
enum UnaryType {
    Unary_PrimaryExp,
    Unary_UnaryExp,
    Unary_NoParam,
    Unary_Param
};

// UnaryExp
class UnaryExpAST : public BaseAST {
  public:
    UnaryType type;
    std::unique_ptr<BaseAST> exp;
    std::string ident;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > funcrparamvec;
    char unaryop;

    std::string DumpIR() const override {
        std::string str;
        std::vector<int> param_ast_i;
        size_t vec_size = 0;
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
            case Unary_NoParam:
                if(global_symbol_table[ident].value == FuncTypeType::FuncType_VOID) {
                    str += "\tcall @";
                    str += ident;
                    str += "()\n";
                }
                else {
                    str += "\t\%";
                    str += std::to_string(ast_i++);
                    str += " = call @";
                    str += ident;
                    str += "()\n";
                }
                break;
            case Unary_Param:
                vec_size = funcrparamvec->size();
                for(int i = vec_size - 1; i > 0; i--) {
                    str += (*funcrparamvec)[i]->DumpIR();
                    param_ast_i.push_back(ast_i - 1);
                }
                str += (*funcrparamvec)[0]->DumpIR();
                param_ast_i.push_back(ast_i - 1);
                if(global_symbol_table[ident].value == FuncTypeType::FuncType_VOID) {
                    str += "\tcall @";
                    str += ident;
                    str += "(";
                    size_t param_vec_size = param_ast_i.size();
                    str += "\%";
                    str += std::to_string(param_ast_i[0]);
                    for(int i = 1; i < param_vec_size; ++i) {
                        str += ", ";
                        str += "\%";
                        str += std::to_string(param_ast_i[i]);
                    }
                    str += ")\n";
                }
                else {
                    str += "\t\%";
                    str += std::to_string(ast_i++);
                    str += " = call @";
                    str += ident;
                    str += "(";
                    size_t param_vec_size = param_ast_i.size();
                    str += "\%";
                    str += std::to_string(param_ast_i[0]);
                    for(int i = 1; i < param_vec_size; ++i) {
                        str += ", ";
                        str += "\%";
                        str += std::to_string(param_ast_i[i]);
                    }
                    str += ")\n";
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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
    
    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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
        int logic_id = 0;
        switch(type) {
            case LAnd_EqExp:
                str = eqexp->DumpIR();
                break;
            case LAnd_ANDExp:
                logic_id = logical_id++;
                str += "\t@result_";
                str += std::to_string(logic_id);
                str += " = alloc i32\n";
                str += landexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = ne \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                str += "\tstore \%";
                str += std::to_string(ast_i - 1);
                str += ", @result_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%then_";
                str += std::to_string(logic_id);
                str += ", \%end_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\%then_";
                str += std::to_string(logic_id);
                str += ":\n";
                str += eqexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = ne \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                str += "\tstore \%";
                str += std::to_string(ast_i - 1);
                str += ", @result_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\tjump \%end_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\%end_";
                str += std::to_string(logic_id);
                str += ":\n";
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = load @result_";
                str += std::to_string(logic_id);
                str += "\n";
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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
        int logic_id = 0;
        switch(type) {
            case LOr_LAndExp:
                str = landexp->DumpIR();
                break;
            case LOr_ORExp:
                logic_id = logical_id++;
                str += "\t@result_";
                str += std::to_string(logic_id);
                str += " = alloc i32\n";
                str += lorexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = ne \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                str += "\tstore \%";
                str += std::to_string(ast_i - 1);
                str += ", @result_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\tbr \%";
                str += std::to_string(ast_i - 1);
                str += ", \%end_";
                str += std::to_string(logic_id);
                str += ", \%then_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\%then_";
                str += std::to_string(logic_id);
                str += ":\n";
                str += landexp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = ne \%";
                str += std::to_string(ast_i - 1);
                str += ", 0\n";
                ast_i++;
                str += "\tstore \%";
                str += std::to_string(ast_i - 1);
                str += ", @result_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\tjump \%end_";
                str += std::to_string(logic_id);
                str += "\n";
                str += "\%end_";
                str += std::to_string(logic_id);
                str += ":\n";
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = load @result_";
                str += std::to_string(logic_id);
                str += "\n";
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// ConstDef Auxiliary data
enum ConstDefType {
    ConstDef_Int,
    ConstDef_Arr
};

// ConstDef
class ConstDefAST : public BaseAST {
  public:
    ConstDefType type;
    std::string ident;
    std::unique_ptr<BaseAST> constinitval;
    std::unique_ptr<BaseAST> constexp;

    std::string DumpIR() const override {
        std::string str;
        std::unique_ptr<std::vector<int> > arr_init;
        int const_val, arr_len, init_len, rest_len;
        switch(type) {
            case ConstDef_Int:
                const_val = ConstCalc();
                if(field == symbol_field::Field_Local)
                    (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                        symbol_t{const_val, symbol_tag::Symbol_Const};
                else
                    global_symbol_table[ident] = symbol_t{const_val, symbol_tag::Symbol_Const};
                break;
            case ConstDef_Arr:
                arr_len = constexp->ConstCalc();
                arr_init = constinitval->getArrInit();
                init_len = arr_init->size();
                rest_len = arr_len - init_len;
                if(field == symbol_field::Field_Local) {
                    (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                        symbol_t{arr_len, symbol_tag::Symbol_Const};
                    str += "\t@";
                    str += ident;
                    str += "_";
                    str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                    str += " = alloc [i32, ";
                    str += std::to_string(arr_len);
                    str += "]\n";
                    for(int i = 0; i < init_len; ++i) {
                        str += "\t\%";
                        str += std::to_string(ast_i);
                        str += " = getelemptr @";
                        str += ident;
                        str += "_";
                        str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                        str += ", ";
                        str += std::to_string(i);
                        str += "\n";
                        str += "\tstore ";
                        str += std::to_string((*arr_init)[i]);
                        str += ", \%";
                        str += std::to_string(ast_i);
                        str += "\n";
                        ast_i++;
                    }
                    for(int i = 0; i < rest_len; ++i) {
                        str += "\t\%";
                        str += std::to_string(ast_i);
                        str += " = getelemptr @";
                        str += ident;
                        str += "_";
                        str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                        str += ", ";
                        str += std::to_string(i + init_len);
                        str += "\n";
                        str += "\tstore ";
                        str += std::to_string(0);
                        str += ", \%";
                        str += std::to_string(ast_i);
                        str += "\n";
                        ast_i++;
                    }
                }
                else {
                    global_symbol_table[ident] = symbol_t{arr_len, symbol_tag::Symbol_Arr};
                    str += "global @";
                    str += ident;
                    str += " = alloc [i32, ";
                    str += std::to_string(arr_len);
                    str += "], {";
                    if(init_len != 0) {
                        str += std::to_string((*arr_init)[0]);
                        for(int i = 1; i < init_len; ++i) {
                            str += ", ";
                            str += std::to_string((*arr_init)[i]);
                        }
                        for(int i = 0; i < rest_len; ++i) {
                            str += ", ";
                            str += std::to_string((*arr_init)[i]);
                        }
                    }
                    else {
                        str += std::to_string(0);
                        for(int i = 1; i < rest_len; ++i) {
                            str += ", ";
                            str += std::to_string(0);
                        }
                    }
                    str += "}\n";
                }
                break;
            default:
                assert(false);
        }
        return str;
    }

    int ConstCalc() const override {
        return constinitval->ConstCalc();
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// ConstInitVal Auxiliary data
enum ConstInitValType {
    ConstInitVal_Exp,
    ConstInitVal_Null,
    ConstInitVal_Vec
};

// ConstInitVal
class ConstInitValAST : public BaseAST {
  public:
    ConstInitValType type;
    std::unique_ptr<BaseAST> constexp;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > constexpvec;

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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        if(type == ConstInitVal_Vec) {
            int vec_size = constexpvec->size();
            for(int i = vec_size - 1; i >= 0; i--)
                arr_ptr->push_back((*constexpvec)[i]->ConstCalc());
        }
        else if(type == ConstInitVal_Exp) {
            std::cerr << "Error: Invalid ConstInitVal.\n";
            assert(false);
        }
        return arr_ptr; 
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
        if(block_ret || block_jump)
            return str;
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// LVal Auxiliary data
enum LValType {
    LVal_Int,
    LVal_Arr
};

// LVal
class LValAST : public BaseAST {
  public:
    LValType type;
    std::string ident;
    std::unique_ptr<BaseAST> exp;

    std::string DumpIR() const override {
        std::string str;
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        symbol_t symbol;
        if(type == LVal_Int) {
            if(target_symbol_table == nullptr) {
                if(global_symbol_table.find(ident) != global_symbol_table.end()) {
                    symbol = global_symbol_table[ident];
                    if(symbol.tag == 
                        symbol_tag::Symbol_Const) {
                        str += "\t\%";
                        str += std::to_string(ast_i);
                        str += " = add 0, ";
                        str += std::to_string(symbol.value);
                        str += "\n";
                        ast_i++;
                    }
                    else if(symbol.tag == symbol_tag::Symbol_Var) {
                        str += "\t\%";
                        str += std::to_string(ast_i);
                        str += " = load @";
                        str += ident;
                        str += "\n";
                        ast_i++;
                    }
                }
                else {
                    std::cerr << "Error: Can't find ident." << std::endl;
                    str = ident;
                }
            }
            else {
                symbol = (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident];
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
                else if(symbol.tag == symbol_tag::Symbol_Var) {
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
        }
        else {
            if(target_symbol_table == nullptr) {
                if(global_symbol_table.find(ident) != global_symbol_table.end()) {
                    str += exp->DumpIR();
                    str += "\t\%";
                    str += std::to_string(ast_i);
                    str += " = getelemptr @";
                    str += ident;
                    str += ", \%";
                    str += std::to_string(ast_i - 1);
                    str += "\n";
                    ast_i++;
                    str += "\t\%";
                    str += std::to_string(ast_i);
                    str += " = load \%";
                    str += std::to_string(ast_i - 1);
                    str += "\n";
                    ast_i++;
                }
                else {
                    std::cerr << "Error: Can't find ident." << std::endl;
                    assert(false);
                }
            }
            else {
                int symbol_num = target_symbol_table->symbol_table_ptr->symbol_table_num;
                str += exp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = getelemptr @";
                str += ident;
                str += "_";
                str += std::to_string(symbol_num);
                str += ", \%";
                str += std::to_string(ast_i - 1);
                str += "\n";
                ast_i++;
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = load \%";
                str += std::to_string(ast_i - 1);
                str += "\n";
                ast_i++;
            }
        }
        return str;
    }

    int ConstCalc() const override {
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        if(target_symbol_table != nullptr)
            return (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident].value;
        return global_symbol_table[ident].value;
    }

    std::string getIdent() const override {
        std::string str;
        str += ident;
        return str;
    }

    std::string getPointer() const override {
        std::string str;
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        if(target_symbol_table == nullptr) {
            if(global_symbol_table.find(ident) != global_symbol_table.end()) {
                str += exp->DumpIR();
                str += "\t\%";
                str += std::to_string(ast_i);
                str += " = getelemptr @";
                str += ident;
                str += ", \%";
                str += std::to_string(ast_i - 1);
                str += "\n";
                ast_i++;
            }
            else {
                std::cerr << "Error: Can't find ident." << std::endl;
                assert(false);
            }
        }
        else {
            int symbol_num = target_symbol_table->symbol_table_ptr->symbol_table_num;
            str += exp->DumpIR();
            str += "\t\%";
            str += std::to_string(ast_i);
            str += " = getelemptr @";
            str += ident;
            str += "_";
            str += std::to_string(symbol_num);
            str += ", \%";
            str += std::to_string(ast_i - 1);
            str += "\n";
            ast_i++;
        }
        return str;
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// VarDecl
class VarDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> functype;
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

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// VarDef Auxiliary data
enum VarDefType {
    VarDef_Int_NO_Init,
    VarDef_Int_Init,
    VarDef_Arr_NO_Init,
    VarDef_Arr_Init
};

// VarDef
class VarDefAST : public BaseAST {
  public:
    VarDefType type;
    std::string ident;
    std::unique_ptr<BaseAST> initval;
    std::unique_ptr<BaseAST> constexp;

    std::string DumpIR() const override {
        std::string str;
        std::unique_ptr<std::vector<int> > arr_init;
        if(type == VarDef_Int_Init || type == VarDef_Int_NO_Init) {
            if(field == symbol_field::Field_Local) {
                str += "\t@";
                str += ident;
                str += "_";
                str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                str += " = alloc i32\n";
                if(type == VarDef_Int_Init) {
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
            }
            else {
                str += "global @";
                str += ident;
                str += " = alloc i32, ";
                if(type == VarDef_Int_Init) {
                    int val = initval->ConstCalc();
                    str += std::to_string(val);
                    str += "\n";
                    global_symbol_table[ident] = symbol_t{val, symbol_tag::Symbol_Var};
                }
                else {
                    str += "zeroinit\n";
                    global_symbol_table[ident] = symbol_t{0, symbol_tag::Symbol_Var};
                }
            }
        }
        else if(type == VarDef_Arr_Init){
            int arr_len = constexp->ConstCalc();
            arr_init = initval->getArrInit();
            int init_len = arr_init->size();
            int rest_len = arr_len - init_len;
            if(field == symbol_field::Field_Local) {
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{arr_len, symbol_tag::Symbol_Arr};
                str += "\t@";
                str += ident;
                str += "_";
                str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                str += " = alloc [i32, ";
                str += std::to_string(arr_len);
                str += "]\n";
                for(int i = 0; i < init_len; ++i) {
                    str += "\t\%";
                    str += std::to_string(ast_i);
                    str += " = getelemptr @";
                    str += ident;
                    str += "_";
                    str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                    str += ", ";
                    str += std::to_string(i);
                    str += "\n";
                    str += "\tstore ";
                    str += std::to_string((*arr_init)[i]);
                    str += ", \%";
                    str += std::to_string(ast_i);
                    str += "\n";
                    ast_i++;
                }
                for(int i = 0; i < rest_len; ++i) {
                    str += "\t\%";
                    str += std::to_string(ast_i);
                    str += " = getelemptr @";
                    str += ident;
                    str += "_";
                    str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                    str += ", ";
                    str += std::to_string(i + init_len);
                    str += "\n";
                    str += "\tstore ";
                    str += std::to_string(0);
                    str += ", \%";
                    str += std::to_string(ast_i);
                    str += "\n";
                    ast_i++;
                }
            }
            else {
                global_symbol_table[ident] = symbol_t{arr_len, symbol_tag::Symbol_Arr};
                str += "global @";
                str += ident;
                str += " = alloc [i32, ";
                str += std::to_string(arr_len);
                str += "], {";
                if(init_len != 0) {
                    str += std::to_string((*arr_init)[0]);
                    for(int i = 1; i < init_len; ++i) {
                        str += ", ";
                        str += std::to_string((*arr_init)[i]);
                    }
                    for(int i = 0; i < rest_len; ++i) {
                        str += ", ";
                        str += std::to_string((*arr_init)[i]);
                    }
                }
                else {
                    str += std::to_string(0);
                    for(int i = 1; i < rest_len; ++i) {
                        str += ", ";
                        str += std::to_string(0);
                    }
                }
                str += "}\n";
            }
        }
        else {
            int arr_len = constexp->ConstCalc();
            if(field == symbol_field::Field_Local) {
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{arr_len, symbol_tag::Symbol_Arr};
                str += "\t@";
                str += ident;
                str += "_";
                str += std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
                str += " = alloc [i32, ";
                str += std::to_string(arr_len);
                str += "]\n";
            }
            else {
                global_symbol_table[ident] = symbol_t{arr_len, symbol_tag::Symbol_Arr};
                str += "global @";
                str += ident;
                str += " = alloc [i32, ";
                str += std::to_string(arr_len);
                str += "], zeroinit\n";
            }
        }
        return str;
    }

    int ConstCalc() const override {
        return 0;
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

// InitVal Auxiliary data
enum InitValType {
    InitVal_Exp,
    InitVal_Null,
    InitVal_Vec
};

// InitVal
class InitValAST : public BaseAST {
  public:
    InitValType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > expvec;

    std::string DumpIR() const override {
        std::string str;
        str += exp->DumpIR();
        return str;
    }

    int ConstCalc() const override {
        return exp->ConstCalc();
    }

    std::string getIdent() const override {
        return std::string();
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit() const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        if(type == InitVal_Vec) {
            int vec_size = expvec->size();
            for(int i = vec_size - 1; i >= 0; i--)
                arr_ptr->push_back((*expvec)[i]->ConstCalc());
        }
        else if(type == InitVal_Exp) {
            std::cerr << "Error: Invalid InitVal.\n";
            assert(false);
        }
        return arr_ptr; 
    }
};