#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <stack>
#include <unordered_map>
#include "symbol.hpp"

/** Variable number */
static int ast_i = 0;
/** Block number */
static unsigned int block_id = 0;
/** Logical op number */
static unsigned int logical_id = 0;
/** While label number */
static unsigned int while_id = 0;
/** Function entry number */
static unsigned int entry_id = 0;
/** True if jump or ret exists inside a block */
static bool block_end = false;
/** True if it's a parameter reference */
static bool is_param_r = false;
/** True if current pointed array's size equal to whole size */
static bool is_all_layer = false;
/** Symbol field */
static symbol_field field = symbol_field::Field_Global;
/** Vector of `while_id` as an iterative struct */
static std::stack<int> while_id_stack;
/** Current local symbol table */
extern symbol_table_list_elem_t *curr_symbol_table;
/** Global symbol table */
extern std::unordered_map<std::string, symbol_t> global_symbol_table;

/**
 * BaseAST is the base of all AST class.
*/
class BaseAST {
  public:
    /**
     * @brief Destroy the BaseAST object.
    */
    virtual ~BaseAST() = default;
    /**
     * @brief Dump AST-specific Koopa IR
    */
    virtual std::string DumpIR() const = 0;
    /**
     * @brief Calculate const value of an expression.
    */
    virtual int ConstCalc() const = 0;
    /**
     * @brief Return `ident` of an AST node.
    */
    virtual std::string getIdent() const = 0;
    /**
     * @brief Return the process of getting value's pointer.
    */
    virtual std::string getPointer() const = 0;
    /**
     * @brief Return array initial value whose blank is filled with 0.
     * @param dim_vec Vector of dimension value.
    */
    virtual std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const = 0;
};

/**
 * CompUnitRootAST is the root node of AST.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/** CompUnit Type Info */
enum CompUnitType {
    CompUnit_SinFunc,
    CompUnit_MulFunc,
    CompUnit_SinDecl,
    CompUnit_MulDecl
};

/** 
 * CompUnitAST represents non-terminal CompUnit.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * FuncDef Type Info.
*/
enum FuncDefType {
    FuncDef_Noparam,
    FuncDef_Param
};

/**
 * FuncDefAST represents non-terminal FuncDef.
*/
class FuncDefAST : public BaseAST {
  public:
    FuncDefType type;
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > funcfparamvec;
    std::unique_ptr<BaseAST> block;

    std::string DumpIR() const override {
        std::string str;
        global_symbol_table[ident] = symbol_t{func_type->ConstCalc(), symbol_tag::Symbol_Func};
        field = symbol_field::Field_Local;
        if(type == FuncDef_Noparam) {
            str += "fun @" + ident + "()" + func_type->DumpIR() + "{\n";
            // %entry_<entry_id> Block
            block_end = false;
            str += "\%entry_" + std::to_string(entry_id++) + ":\n";
            str += block->DumpIR();
            if(!block_end) {
                if(func_type->ConstCalc() == 0)
                    str += "\tret 0\n";
                else
                    str += "\tret\n";
                block_end = true;
            }
            str += "}\n";
        }
        else {
            str += "fun @" + ident + "(";
            create_symbol_table();
            size_t vec_size = funcfparamvec->size();
            for(int i = vec_size - 1; i > 0; i--) {
                str += (*funcfparamvec)[i]->getIdent();
                str += ", ";
            }
            str += (*funcfparamvec)[0]->getIdent() + ")" + func_type->DumpIR() + "{\n";
            // %entry_<entry_id> Block
            block_end = false;
            str += "\%entry_" + std::to_string(entry_id++) + ":\n";
            for(int i = vec_size - 1; i >= 0; i--)
                str += (*funcfparamvec)[i]->DumpIR();
            str += block->DumpIR();
            if(!block_end) {
                if(func_type->ConstCalc() == 0)
                    str += "\tret 0\n";
                else
                    str += "\tret\n";
                block_end = true;
            }
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * FuncType Type Info.
*/
enum FuncTypeType {
    FuncType_INT,
    FuncType_VOID
};

/**
 * FuncTypeAST represents non-terminal FuncType.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * FuncFParam Type Info.
*/
enum FuncFParamType {
    FuncFParam_Int,
    FuncFParam_Arr_Sin,
    FuncFParam_Arr_Mul
};

/**
 * FuncFParamAST represents non-terminal FuncFParam.
*/
class FuncFParamAST : public BaseAST {
  public:
    FuncFParamType type;
    std::unique_ptr<BaseAST> btype;
    std::string ident;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > >constexpvec;

    std::string DumpIR() const override {
        std::string str;
        std::string param_ident = std::string("param_") + ident + "_" +
                        std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num);
        int vec_size;
        std::vector<int> dim_vec;
        std::string dim_str;
        switch(type) {
            case FuncFParam_Int:
                str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + " = alloc i32\n\tstore @" + param_ident + ", @" + ident + "_"
                     + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num) + "\n";
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{-1, symbol_tag::Symbol_Var};
                break;
            case FuncFParam_Arr_Sin:
                str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + " = alloc *i32\n\tstore @" + param_ident + ", @" + ident + "_"
                     + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num) + "\n";
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{1, symbol_tag::Symbol_Pointer};
                break;
            case FuncFParam_Arr_Mul:
                vec_size = (*constexpvec).size();
                for(int i = 0; i < vec_size; ++i) {
                    int dim = (*constexpvec)[i]->ConstCalc();
                    dim_vec.push_back(dim);
                }
                str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + " = alloc *";
                dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                for(int i = 1; i < dim_vec.size(); ++i) 
                    dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                str += dim_str + "\n" + "\tstore @" + param_ident + ", @" + ident + "_"
                     + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num) + "\n";
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{(int)(dim_vec.size()) + 1, symbol_tag::Symbol_Pointer};
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
        std::string str;
        int vec_size;
        std::vector<int> dim_vec;
        std::string dim_str;
        switch(type) {
            case FuncFParam_Int:
                str += "@param_" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + ": i32";
                break;
            case FuncFParam_Arr_Sin:
                str += "@param_" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + ": *i32";
                break;
            case FuncFParam_Arr_Mul:
                vec_size = (*constexpvec).size();
                for(int i = 0; i < vec_size; ++i) {
                    int dim = (*constexpvec)[i]->ConstCalc();
                    dim_vec.push_back(dim);
                }
                str += "@param_" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + ": *";
                dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                for(int i = 1; i < dim_vec.size(); ++i) 
                    dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                str += dim_str;
                break;
            default:
                assert(false);
        }
        return str;
    }

    std::string getPointer() const override {
        return std::string();
    }

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Block Type Info.
*/
enum BlockType {
    Block_Items,
    Block_Null
};

/**
 * BlockAST represents non-terminal Block.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Stmt Type Info.
*/
enum StmtType {
    Stmt_Open,
    Stmt_Closed
};

/**
 * StmtAST represents non-terminal Stmt.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * OpenStmt Type Info.
*/
enum OpenStmtType {
    Open_If,
    Open_Else,
    Open_While
};

/**
 * OpenStmtAST represents non-terminal OpenStmt.
*/
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
        switch(type) {
            case Open_If:
                then_id = block_id++;
                end_id = block_id++;
                str += exp->DumpIR() + "\tbr \%" + std::to_string(ast_i - 1) + ", \%block_"
                     + std::to_string(then_id) + ", \%block_" + std::to_string(end_id) + "\n";
                // %block_<then_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(then_id) + ":\n";
                str += stmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%block_" + std::to_string(end_id) + "\n";
                    block_end = true;
                }
                // %block_<end_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(end_id) + ":\n";
                break;
            case Open_Else:
                then_id = block_id++;
                else_id = block_id++;
                end_id = block_id++;
                str += exp->DumpIR() + "\tbr \%" + std::to_string(ast_i - 1) + ", \%block_"
                     + std::to_string(then_id) + ", \%block_" + std::to_string(else_id) + "\n";
                // %block_<then_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(then_id) + ":\n";
                str += stmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%block_" + std::to_string(end_id) + "\n";
                    block_end = true;
                }
                // %block_<else_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(else_id) + ":\n";
                str += openstmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%block_" + std::to_string(end_id) + "\n";
                    block_end = true;
                }
                // %block_<end_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(end_id) + ":\n";
                break;
            case Open_While:
                while_id_stack.push(while_id++);
                while_block_id = while_id_stack.top();
                str += "\tjump \%while_entry_" + std::to_string(while_block_id) + "\n";
                // %while_entry_<while_blcok_id> Block
                block_end = false;
                str += "\%while_entry_" + std::to_string(while_block_id) + ":\n";
                str += exp->DumpIR() + "\tbr \%" + std::to_string(ast_i - 1) + ", \%while_body_"
                     + std::to_string(while_block_id) + ", \%while_end_" + std::to_string(while_block_id) + "\n";
                // %while_body_<while_block_id> Block
                block_end = false;
                str += "\%while_body_" + std::to_string(while_block_id) + ":\n";
                str += openstmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%while_entry_" + std::to_string(while_block_id) + "\n";
                    block_end = true;
                }
                // %while_end_<while_block_id> Block
                block_end = false;
                str += "\%while_end_" + std::to_string(while_block_id) + ":\n";
                while_id_stack.pop();
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * ClosedStmt Type Info.
*/
enum ClosedStmtType {
    Closed_NonIf,
    Closed_If,
    Closed_While
};

/**
 * ClosedStmtAST represents non-terminal ClosedStmt.
*/
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
        switch(type) {
            case Closed_NonIf:
                str += stmt->DumpIR();
                break;
            case Closed_If:
                then_id = block_id++;
                else_id = block_id++;
                end_id = block_id++;
                str += exp->DumpIR() + "\tbr \%" + std::to_string(ast_i - 1) + ", \%block_" + std::to_string(then_id)
                     + ", \%block_" + std::to_string(else_id) + "\n";
                // %block_<then_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(then_id) + ":\n";
                str += stmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%block_" + std::to_string(end_id) + "\n";
                    block_end = true;
                }
                // %block_<else_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(else_id) + ":\n";
                str += closedstmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%block_" + std::to_string(end_id) + "\n";
                    block_end = true;
                }
                // %block_<end_id> Block
                block_end = false;
                str += "\%block_" + std::to_string(end_id) + ":\n";
                break;
            case Closed_While:
                while_id_stack.push(while_id++);
                while_block_id = while_id_stack.top();
                str += "\tjump \%while_entry_" + std::to_string(while_block_id) + "\n";
                // %while_entry_<while_blcok_id> Block
                block_end = false;
                str += "\%while_entry_" + std::to_string(while_block_id) + ":\n";
                str += exp->DumpIR() + "\tbr \%" + std::to_string(ast_i - 1) + ", \%while_body_" + std::to_string(while_block_id)
                     + ", \%while_end_" + std::to_string(while_block_id) + "\n";
                // %while_body_<while_block_id> Block
                block_end = false;
                str += "\%while_body_" + std::to_string(while_block_id) + ":\n";
                str += stmt->DumpIR();
                if(!block_end) {
                    str += "\tjump \%while_entry_" + std::to_string(while_block_id) + "\n";
                    block_end = true;
                }
                // %while_end_<while_block_id> Block
                block_end = false;
                str += "\%while_end_" + std::to_string(while_block_id) + ":\n";
                while_id_stack.pop();
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * NonIfStmt Type Info.
*/
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

/**
 * NonIfStmtAST represents non-terminal NonIfStmt.
*/
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
                str += exp->DumpIR() + "\tret \%" + std::to_string(ast_i - 1) + "\n";
                block_end = true;
                break;
            case NonIf_Lval:
                str += exp->DumpIR();
                store_src = ast_i - 1;
                ident = lval->getIdent();
                target_symbol_table = search_symbol_table(ident);   
                if(target_symbol_table != nullptr) {
                    tag = (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident].tag;
                    ident += "_" + std::to_string(target_symbol_table->symbol_table_ptr->symbol_table_num);
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
                        str += "\tstore \%" + std::to_string(ast_i - 1) + ", @" + ident + "\n";
                        break;
                    case Symbol_Arr:
                        str += lval->getPointer() + "\tstore \%" + std::to_string(store_src) + ", \%"
                             + std::to_string(ast_i - 1) + "\n";
                        break;
                    case Symbol_Pointer:
                        str += lval->getPointer() + "\tstore \%" + std::to_string(store_src) + ", \%"
                             + std::to_string(ast_i - 1) + "\n";
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
                block_end = true;
                break;
            case NonIf_Break:
                if(while_id_stack.empty())
                    break;
                while_block_id = while_id_stack.top();
                if(!block_end) {
                    str += "\tjump \%while_end_" + std::to_string(while_block_id) + "\n";
                    block_end = true;
                }
                break;
            case NonIf_Continue:
                if(while_id_stack.empty())
                    break;
                while_block_id = while_id_stack.top();
                if(!block_end) {
                    str += "\tjump \%while_entry_" + std::to_string(while_block_id) + "\n";
                    block_end = true;
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * ExpAST represents non-terminal Exp.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Primary Type Info.
*/
enum PrimaryType {
    Primary_Exp,
    Primary_Number,
    Primary_Lval
};

/**
 * PrimaryExpAST represents non-terminal PrimaryExp.
*/
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
                str += "\t\%" + std::to_string(ast_i) + " = add 0, " + std::to_string(number) + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Unary Type Info.
*/
enum UnaryType {
    Unary_PrimaryExp,
    Unary_UnaryExp,
    Unary_NoParam,
    Unary_Param
};

/**
 * UnaryExpAST represents non-terminal UnaryExp.
*/
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
                    str += "\t\%" + std::to_string(ast_i) + " = sub 0, \%" + std::to_string(ast_i - 1) + "\n";
                    ast_i++;
                }
                else if(unaryop == '!'){
                    str += "\t\%" + std::to_string(ast_i) + " = eq \%" + std::to_string(ast_i - 1) + ", 0\n";
                    ast_i++;
                }
                break;
            case Unary_NoParam:
                if(global_symbol_table[ident].value == FuncTypeType::FuncType_VOID)
                    str += "\tcall @" + ident + "()\n";
                else
                    str += "\t\%" + std::to_string(ast_i++) + " = call @" + ident + "()\n";
                break;
            case Unary_Param:
                vec_size = funcrparamvec->size();
                is_param_r = true;
                for(int i = vec_size - 1; i > 0; i--) {
                    str += (*funcrparamvec)[i]->DumpIR();
                    param_ast_i.push_back(ast_i - 1);
                }
                str += (*funcrparamvec)[0]->DumpIR();
                is_param_r = false;
                param_ast_i.push_back(ast_i - 1);
                if(global_symbol_table[ident].value == FuncTypeType::FuncType_VOID) {
                    str += "\tcall @" + ident + "(\%";
                    size_t param_vec_size = param_ast_i.size();
                    str += std::to_string(param_ast_i[0]);
                    for(int i = 1; i < param_vec_size; ++i)
                        str += ", \%" + std::to_string(param_ast_i[i]);
                    str += ")\n";
                }
                else {
                    str += "\t\%" + std::to_string(ast_i++) + " = call @" + ident + "(\%";
                    size_t param_vec_size = param_ast_i.size();
                    str += std::to_string(param_ast_i[0]);
                    for(int i = 1; i < param_vec_size; ++i)
                        str += ", \%" + std::to_string(param_ast_i[i]);
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Mul Type Info.
*/
enum MulType {
    Mul_UnaryExp,
    Mul_MulExp,
    Mul_DivExp,
    Mul_ModExp
};

/**
 * MulExpAST represents non-terminal MulExp.
*/
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
                str += mulexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = mul \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Mul_DivExp:
                str = unaryexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += mulexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = div \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Mul_ModExp:
                str = unaryexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += mulexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = mod \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Add Type Info.
*/
enum AddType {
    Add_MulExp,
    Add_AddExp,
    Add_MinusExp
};

/**
 * AddExpAST represents non-terminal AddExp.
*/
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
                str += addexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = add \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Add_MinusExp:
                str = mulexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += addexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = sub \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * RelExp Type Info.
*/
enum RelType {
    Rel_AddExp,
    Rel_LTExp,
    Rel_GTExp,
    Rel_LEExp,
    Rel_GEExp
};

/**
 * RelExpAST represents non-terminal RelExp.
*/
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
                str += relexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = lt \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Rel_GTExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = gt \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Rel_LEExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = le \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Rel_GEExp:
                str = addexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += relexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = ge \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * EqExp Type Info.
*/
enum EqType {
    Eq_RelExp,
    Eq_EQExp,
    Eq_NEQExp
};

/**
 * EqExpAST represents non-terminal EqExp.
*/
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
                str += eqexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = eq \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
                ast_i++;
                break;
            case Eq_NEQExp:
                str = relexp->DumpIR();
                r_num = std::to_string(ast_i - 1);
                str += eqexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = ne \%" + std::to_string(ast_i - 1)
                     + ", \%" + r_num + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * LAndExp Type Info.
*/
enum LAndType {
    LAnd_EqExp,
    LAnd_ANDExp
};

/**
 * LAndExpAST represents non-terminal LAndExp.
*/
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
                str += "\t@result_" + std::to_string(logic_id) + " = alloc i32\n" + landexp->DumpIR() + "\t\%"
                     + std::to_string(ast_i) + " = ne \%" + std::to_string(ast_i - 1) + ", 0\n";
                ast_i++;
                str += "\tstore \%" + std::to_string(ast_i - 1) + ", @result_" + std::to_string(logic_id) + "\n";
                str += "\tbr \%" + std::to_string(ast_i - 1) + ", \%then_" + std::to_string(logic_id) + ", \%end_" + std::to_string(logic_id) + "\n";
                // %then_<logic_id> Block
                block_end = false;
                str += "\%then_" + std::to_string(logic_id) + ":\n";
                str += eqexp->DumpIR() + "\t\%" + std::to_string(ast_i)
                     + " = ne \%" + std::to_string(ast_i - 1) + ", 0\n";
                ast_i++;
                str += "\tstore \%" + std::to_string(ast_i - 1) + ", @result_" + std::to_string(logic_id) + "\n";
                str += "\tjump \%end_" + std::to_string(logic_id) + "\n";
                // %end_<logic_id> Block
                block_end = false;
                str += "\%end_" + std::to_string(logic_id) + ":\n";
                str += "\t\%" + std::to_string(ast_i) + " = load @result_" + std::to_string(logic_id) + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * LOrExp Type Info.
*/
enum LOrType {
    LOr_LAndExp,
    LOr_ORExp
};

/**
 * LOrExpAST represents non-terminal LOrExp.
*/
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
                str += "\t@result_" + std::to_string(logic_id) + " = alloc i32\n" + lorexp->DumpIR() + "\t\%"
                     + std::to_string(ast_i) + " = ne \%" + std::to_string(ast_i - 1) + ", 0\n";
                ast_i++;
                str += "\tstore \%" + std::to_string(ast_i - 1) + ", @result_" + std::to_string(logic_id) + "\n";
                str += "\tbr \%" + std::to_string(ast_i - 1) + ", \%end_" + std::to_string(logic_id) + ", \%then_" + std::to_string(logic_id) + "\n";
                // %then_<logic_id> Block
                block_end = false;
                str += "\%then_" + std::to_string(logic_id) + ":\n";
                str += landexp->DumpIR() + "\t\%" + std::to_string(ast_i) + " = ne \%" + std::to_string(ast_i - 1) + ", 0\n";
                ast_i++;
                str += "\tstore \%" + std::to_string(ast_i - 1) + ", @result_" + std::to_string(logic_id) + "\n";
                str += "\tjump \%end_" + std::to_string(logic_id) + "\n";
                // %end_<logic_id> Block
                block_end = false;
                str += "\%end_" + std::to_string(logic_id) + ":\n";
                str += "\t\%" + std::to_string(ast_i) + " = load @result_" + std::to_string(logic_id) + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Decl Type Info.
*/
enum DeclType {
    Decl_Const,
    Decl_Var
};

/**
 * DeclAST represents non-terminal Decl.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * ConstDeclAST represents non-terminal ConstDecl.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * BTypeAST represents non-terminal BType.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * ConstDef Type Info.
*/
enum ConstDefType {
    ConstDef_Int,
    ConstDef_Arr
};

/**
 * ConstDefAST represents non-terminal ConstDef.
*/
class ConstDefAST : public BaseAST {
  public:
    ConstDefType type;
    std::string ident;
    std::unique_ptr<BaseAST> constinitval;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > constexpvec;

    std::string DumpIR() const override {
        std::string str;
        std::unique_ptr<std::vector<int> > arr_init;
        int const_val, arr_len, vec_size;
        std::vector<int> dim_vec;
        int elem_num;
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
                vec_size = (*constexpvec).size();
                arr_len = 1;
                for(int i = 0; i < vec_size; ++i) {
                    int dim = (*constexpvec)[i]->ConstCalc();
                    dim_vec.push_back(dim);
                    arr_len *= dim;
                }
                arr_init = constinitval->getArrInit(dim_vec, str);
                elem_num = arr_init->size();
                for(int _ = elem_num; _ < arr_len; ++_) arr_init->push_back(0);
                if(field == symbol_field::Field_Local) {
                    (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                        symbol_t{(int)(dim_vec.size()), symbol_tag::Symbol_Arr};
                    str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num) + " = alloc ";
                    std::string dim_str;
                    dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                    for(int i = 1; i < dim_vec.size(); ++i) 
                        dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                    str += dim_str + "\n";
                    for(int i = 0; i < arr_len; ++i) {
                        std::vector<int> arr_index;
                        int index = i;
                        int layer_size = arr_len / dim_vec[vec_size - 1];
                        int k = 0;
                        while(k < vec_size - 1) {
                            arr_index.push_back(index / layer_size);
                            index = index % layer_size;
                            layer_size = layer_size / dim_vec[vec_size - 1 - (++k)];
                        }
                        arr_index.push_back(index);
                        int arr_index_size = arr_index.size();
                        str += "\t\%" + std::to_string(ast_i++) + " = getelemptr @" + ident + "_"
                             + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num) + ", "
                             + std::to_string(arr_index[0]) + "\n";
                        for(int j = 1; j < arr_index_size; ++j) {
                            str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1)
                                 + ", " + std::to_string(arr_index[j]) + "\n";
                            ast_i++;
                        }
                        str += "\tstore " + std::to_string((*arr_init)[i]) + ", \%" + std::to_string(ast_i - 1) + "\n";
                    }
                }
                else {
                    global_symbol_table[ident] = symbol_t{(int)dim_vec.size(), symbol_tag::Symbol_Arr};
                    str += "global @" + ident + " = alloc ";
                    std::string dim_str;
                    dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                    for(int i = 1; i < dim_vec.size(); ++i) 
                        dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                    str += dim_str + ", ";
                    std::vector<std::string> init_str_vec;
                    std::vector<std::string> compress_init_str_vec;
                    for(int i : (*arr_init)) init_str_vec.push_back(std::to_string(i));
                    for(int i = 0; i < vec_size; ++i) {
                        compress_init_str_vec.clear();
                        int init_str_vec_size = init_str_vec.size();
                        int index = 0;
                        while(index < init_str_vec_size) {
                            std::string init_str_set;
                            init_str_set += "{" + init_str_vec[index];
                            for(int j = 1; j < dim_vec[i]; ++j)
                                init_str_set += ", " + init_str_vec[index + j];
                            init_str_set += "}";
                            index += dim_vec[i];
                            compress_init_str_vec.push_back(init_str_set);
                        }
                        init_str_vec = compress_init_str_vec;
                    }
                    str += init_str_vec[0] + "\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * ConstInitVal Type Info.
*/
enum ConstInitValType {
    ConstInitVal_Exp,
    ConstInitVal_Null,
    ConstInitVal_Vec
};

/**
 * ConstInitValAST represents non-terminal ConstInitVal.
*/
class ConstInitValAST : public BaseAST {
  public:
    ConstInitValType type;
    std::unique_ptr<BaseAST> constexp;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > constinitvalvec;

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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        int arr_size = 1;
        int elem_num = 0;
        for(int i : dim_vec) arr_size *= i;
        if(type == ConstInitVal_Vec) {
            int vec_size = constinitvalvec->size();
            int dim_vec_size = dim_vec.size();
            std::vector<int> new_dim_vec;
            for(int i = vec_size - 1; i >= 0; i--) {
                new_dim_vec.clear();
                ConstInitValAST *constinitval = reinterpret_cast<ConstInitValAST *>
                                                ((*constinitvalvec)[i].get());
                if(constinitval->type == ConstInitValType::ConstInitVal_Vec ||
                  constinitval->type == ConstInitValType::ConstInitVal_Null) {
                    if(elem_num == 0) {
                        new_dim_vec = dim_vec;
                        new_dim_vec.pop_back();
                    }
                    else {
                        int layer = 0;
                        int layer_size = 1;
                        while(layer < dim_vec_size) {
                            if(elem_num % layer_size == 0) {
                                new_dim_vec.push_back(dim_vec[layer]);
                                layer_size *= dim_vec[layer];
                                layer++;
                            }
                            else break;
                        }
                        new_dim_vec.pop_back();
                    }
                    if(new_dim_vec.empty()) {
                        std::cerr << "Error: Invalid Array.\n";
                        assert(false);
                    }
                    std::unique_ptr<std::vector<int> > new_arr = constinitval->getArrInit(new_dim_vec, str);
                    arr_ptr->insert(arr_ptr->end(), new_arr->begin(), new_arr->end());
                    elem_num = arr_ptr->size();
                }
                else if(constinitval->type == ConstInitValType::ConstInitVal_Exp) {
                    arr_ptr->push_back(constinitval->ConstCalc());
                    elem_num++;
                    continue;
                }
            }
        }
        else if(type == ConstInitVal_Exp) {
            std::cerr << "Error: Invalid ConstInitVal.\n";
            assert(false);
        }
        if(elem_num < arr_size) {
            for(int _ = elem_num; _ < arr_size; ++_) arr_ptr->push_back(0);
        }
        return arr_ptr; 
    }
};

/**
 * BlockItem Type Info.
*/
enum BlockItemType {
    BlockItem_Decl,
    BlockItem_Stmt
};

/**
 * BlockItemAST represents non-terminal BlockItem.
*/
class BlockItemAST : public BaseAST {
  public:
    BlockItemType type;
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;

    std::string DumpIR() const override {
        std::string str;
        if(block_end)
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * Lval Type Info.
*/
enum LValType {
    LVal_Int,
    LVal_Arr
};

/**
 * LValAST represents non-terminal LVal.
*/
class LValAST : public BaseAST {
  public:
    LValType type;
    std::string ident;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > expvec;

    std::string DumpIR() const override {
        std::string str;
        symbol_table_list_elem_t *target_symbol_table = search_symbol_table(ident);
        symbol_t symbol;
        if(type == LVal_Int) {
            if(target_symbol_table == nullptr) {
                if(global_symbol_table.find(ident) != global_symbol_table.end()) {
                    symbol = global_symbol_table[ident];
                    if(symbol.tag == symbol_tag::Symbol_Const)
                        str += "\t\%" + std::to_string(ast_i) + " = add 0, " + std::to_string(symbol.value) + "\n";
                    else if(symbol.tag == symbol_tag::Symbol_Var)
                        str += "\t\%" + std::to_string(ast_i) + " = load @" + ident + "\n";
                    else if(symbol.tag == symbol_tag::Symbol_Arr)
                        str += "\t\%" + std::to_string(ast_i) + " = getelemptr @" + ident + ", 0\n";
                    else
                        str += "\t\%" + std::to_string(ast_i) + " = load @" + ident + "\n";
                    ast_i++;
                }
                else {
                    std::cerr << "Error: Can't find ident." << std::endl;
                    str = ident;
                }
            }
            else {
                symbol = (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident];
                int symbol_num = target_symbol_table->symbol_table_ptr->symbol_table_num;
                if(symbol.tag == symbol_tag::Symbol_Const)
                    str += "\t\%" + std::to_string(ast_i) + " = add 0, " + std::to_string(symbol.value) + "\n";
                else if(symbol.tag == symbol_tag::Symbol_Var)
                    str += "\t\%" + std::to_string(ast_i) + " = load @" + ident + "_" + std::to_string(symbol_num) + "\n";
                else if(symbol.tag == symbol_tag::Symbol_Arr)
                    str += "\t\%" + std::to_string(ast_i) + " = getelemptr @" + ident + "_" + std::to_string(symbol_num) + ", 0\n";
                else
                    str += "\t\%" + std::to_string(ast_i) + " = load @" + ident + "_" + std::to_string(symbol_num) + "\n";
                ast_i++;
            }
        }
        else {
            if(target_symbol_table == nullptr) {
                if(global_symbol_table.find(ident) != global_symbol_table.end()) {
                    str += getPointer();
                    if(is_param_r && !is_all_layer)
                        str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1) + ", 0\n";
                    else
                        str += "\t\%" + std::to_string(ast_i) + " = load \%" + std::to_string(ast_i - 1) + "\n";
                    ast_i++;
                }
                else {
                    std::cerr << "Error: Can't find ident." << std::endl;
                    assert(false);
                }
            }
            else {
                str += getPointer();
                if(is_param_r && !is_all_layer)
                    str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1) + ", 0\n";
                else
                    str += "\t\%" + std::to_string(ast_i) + " = load \%" + std::to_string(ast_i - 1) + "\n";
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
                symbol_tag tag = global_symbol_table[ident].tag;
                int expvec_size = expvec->size();
                std::vector<int> index_vec;
                for(int i = expvec_size - 1; i >= 0; i--) {
                    str += (*expvec)[i]->DumpIR();
                    index_vec.push_back(ast_i - 1);
                }
                int index_vec_size = index_vec.size();
                if(expvec_size == global_symbol_table[ident].value)
                    is_all_layer = true;
                else
                    is_all_layer = false;
                if(tag == Symbol_Arr){
                    str += "\t\%" + std::to_string(ast_i) + " = getelemptr @" + ident + ", \%" + std::to_string(index_vec[0]) + "\n";
                    ast_i++;
                    for(int i = 1; i < index_vec_size; ++i) {
                        str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1) + ", \%"
                             + std::to_string(index_vec[i]) + "\n";
                        ast_i++;
                    }
                }
                else if(tag == Symbol_Pointer) {
                    str += "\t\%" + std::to_string(ast_i++);
                    str += " = load @" + ident + "\n\t\%" + std::to_string(ast_i)
                         + " = getptr \%" + std::to_string(ast_i - 1) + ", \%" + std::to_string(index_vec[0]) + "\n";
                    ast_i++;
                    for(int i = 1; i < index_vec_size; ++i) {
                        str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1) + ", \%"
                             + std::to_string(index_vec[i]) + "\n";
                        ast_i++;
                    }
                }
            }
            else {
                std::cerr << "Error: Can't find ident." << std::endl;
                assert(false);
            }
        }
        else {
            symbol_tag tag = (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident].tag;
            int symbol_num = target_symbol_table->symbol_table_ptr->symbol_table_num;
            int expvec_size = expvec->size();
            std::vector<int> index_vec;
            for(int i = expvec_size - 1; i >= 0; i--) {
                str += (*expvec)[i]->DumpIR();
                index_vec.push_back(ast_i - 1);
            }
            int index_vec_size = index_vec.size();
            if(expvec_size == (*(target_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident].value)
                is_all_layer = true;
            else
                is_all_layer = false;
            if(tag == Symbol_Arr) {
                str += "\t\%" + std::to_string(ast_i) + " = getelemptr @" + ident + "_" + std::to_string(symbol_num)
                     + ", \%" + std::to_string(index_vec[0]) + "\n";
                ast_i++;
                for(int i = 1; i < index_vec_size; ++i) {
                    str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1)
                         + ", \%" + std::to_string(index_vec[i]) + "\n";
                    ast_i++;
                }
            }
            else if(tag == Symbol_Pointer) {
                str += "\t\%" + std::to_string(ast_i++);
                str += " = load @" + ident + "_" + std::to_string(symbol_num)
                     + "\n\t\%" + std::to_string(ast_i) + " = getptr \%" + std::to_string(ast_i - 1) + ", \%"
                     + std::to_string(index_vec[0]) + "\n";
                ast_i++;
                for(int i = 1; i < index_vec_size; ++i) {
                    str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1) + ", \%"
                         + std::to_string(index_vec[i]) + "\n";
                    ast_i++;
                }
            }
        }
        return str;
    }

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * ConstExpAST represents non-terminal ConstExp.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * VarDeclAST represents non-terminal VarDecl.
*/
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * VarDef Type Info.
*/
enum VarDefType {
    VarDef_Int_NO_Init,
    VarDef_Int_Init,
    VarDef_Arr_NO_Init,
    VarDef_Arr_Init
};

/**
 * VarDefAST represents non-terminal VarDef.
*/
class VarDefAST : public BaseAST {
  public:
    VarDefType type;
    std::string ident;
    std::unique_ptr<BaseAST> initval;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > constexpvec;

    std::string DumpIR() const override {
        std::string str;
        std::unique_ptr<std::vector<int> > arr_init;
        if(type == VarDef_Int_Init || type == VarDef_Int_NO_Init) {
            if(field == symbol_field::Field_Local) {
                str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + " = alloc i32\n";
                if(type == VarDef_Int_Init)
                    str += initval->DumpIR() + "\tstore \%" + std::to_string(ast_i - 1) + ", @" + ident + "_"
                         + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num) + "\n";
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{-1, symbol_tag::Symbol_Var};
            }
            else {
                str += "global @" + ident + " = alloc i32, ";
                if(type == VarDef_Int_Init) {
                    int val = initval->ConstCalc();
                    str += std::to_string(val) + "\n";
                    global_symbol_table[ident] = symbol_t{val, symbol_tag::Symbol_Var};
                }
                else {
                    str += "zeroinit\n";
                    global_symbol_table[ident] = symbol_t{0, symbol_tag::Symbol_Var};
                }
            }
        }
        else if(type == VarDef_Arr_Init){
            int vec_size = (*constexpvec).size();
            int arr_len = 1;
            std::vector<int> dim_vec;
            for(int i = 0; i < vec_size; ++i) {
                int dim = (*constexpvec)[i]->ConstCalc();
                dim_vec.push_back(dim);
                arr_len *= dim;
            }
            arr_init = initval->getArrInit(dim_vec, str);
            int elem_num = arr_init->size();
            if(elem_num < arr_len) {
                if(field == symbol_field::Field_Local)
                    for(int _ = elem_num; _ < arr_len; ++_) arr_init->push_back(-1);
                else
                    for(int _ = elem_num; _ < arr_len; ++_) arr_init->push_back(0);
            }
            if(field == symbol_field::Field_Local) {
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{(int)(dim_vec.size()), symbol_tag::Symbol_Arr};
                str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + " = alloc ";
                std::string dim_str;
                dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                for(int i = 1; i < dim_vec.size(); ++i) 
                    dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                str += dim_str + "\n";
                for(int i = 0; i < arr_len; ++i) {
                    std::vector<int> arr_index;
                    int index = i;
                    int layer_size = arr_len / dim_vec[vec_size - 1];
                    int k = 0;
                    while(k < vec_size - 1) {
                        arr_index.push_back(index / layer_size);
                        index = index % layer_size;
                        layer_size = layer_size / dim_vec[vec_size - 1 - (++k)];
                    }
                    arr_index.push_back(index);
                    int arr_index_size = arr_index.size();
                    str += "\t\%" + std::to_string(ast_i++);
                    str += " = getelemptr @" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                         + ", " + std::to_string(arr_index[0]) + "\n";
                    for(int j = 1; j < arr_index_size; ++j) {
                        str += "\t\%" + std::to_string(ast_i) + " = getelemptr \%" + std::to_string(ast_i - 1) + ", "
                             + std::to_string(arr_index[j]) + "\n";
                        ast_i++;
                    }
                    if((*arr_init)[i] != -1)
                        str += "\tstore \%" + std::to_string((*arr_init)[i]) + ", \%" + std::to_string(ast_i - 1) + "\n";
                    else
                        str += "\tstore 0, \%" + std::to_string(ast_i - 1) + "\n";
                }
            }
            else {
                global_symbol_table[ident] = symbol_t{(int)(dim_vec.size()), symbol_tag::Symbol_Arr};
                str += "global @" + ident + " = alloc ";
                std::string dim_str;
                dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                for(int i = 1; i < dim_vec.size(); ++i) 
                    dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                str += dim_str + ", ";
                std::vector<std::string> init_str_vec;
                std::vector<std::string> compress_init_str_vec;
                for(int i : (*arr_init)) init_str_vec.push_back(std::to_string(i));
                for(int i = 0; i < vec_size; ++i) {
                    compress_init_str_vec.clear();
                    int init_str_vec_size = init_str_vec.size();
                    int index = 0;
                    while(index < init_str_vec_size) {
                        std::string init_str_set;
                        init_str_set += "{" + init_str_vec[index];
                        for(int j = 1; j < dim_vec[i]; ++j)
                            init_str_set += ", " + init_str_vec[index + j];
                        init_str_set += "}";
                        index += dim_vec[i];
                        compress_init_str_vec.push_back(init_str_set);
                    }
                    init_str_vec = compress_init_str_vec;
                }
                str += init_str_vec[0] + "\n";
            }
        }
        else {
            int vec_size = (*constexpvec).size();
            std::vector<int> dim_vec;
            for(int i = 0; i < vec_size; ++i) {
                int dim = (*constexpvec)[i]->ConstCalc();
                dim_vec.push_back(dim);
            }
            if(field == symbol_field::Field_Local) {
                (*(curr_symbol_table->symbol_table_ptr->symbol_table_elem_ptr))[ident] = 
                    symbol_t{(int)(dim_vec.size()), symbol_tag::Symbol_Arr};
                str += "\t@" + ident + "_" + std::to_string(curr_symbol_table->symbol_table_ptr->symbol_table_num)
                     + " = alloc ";
                std::string dim_str;
                dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                for(int i = 1; i < dim_vec.size(); ++i) 
                    dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                str += dim_str + "\n";
            }
            else {
                global_symbol_table[ident] = symbol_t{(int)(dim_vec.size()), symbol_tag::Symbol_Arr};
                str += "global @" + ident + " = alloc ";
                std::string dim_str;
                dim_str += ("[i32, " + std::to_string(dim_vec[0]) + "]");
                for(int i = 1; i < dim_vec.size(); ++i) 
                    dim_str = "[" + dim_str + ", " + std::to_string(dim_vec[i]) + "]";
                str += dim_str + ", zeroinit\n";
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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        return arr_ptr; 
    }
};

/**
 * InitVal Type Info.
*/
enum InitValType {
    InitVal_Exp,
    InitVal_Null,
    InitVal_Vec
};

/**
 * InitValAST represents non-terminal InitVal.
*/
class InitValAST : public BaseAST {
  public:
    InitValType type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<std::vector<std::unique_ptr<BaseAST> > > initvalvec;

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

    std::unique_ptr<std::vector<int> > getArrInit(std::vector<int> dim_vec, std::string& str) const override {
        std::unique_ptr<std::vector<int> > arr_ptr = std::make_unique<std::vector<int> >();
        int arr_size = 1;
        int elem_num = 0;
        for(int i : dim_vec) arr_size *= i;
        if(type == InitVal_Vec) {
            int vec_size = initvalvec->size();
            int dim_vec_size = dim_vec.size();
            std::vector<int> new_dim_vec;
            for(int i = vec_size - 1; i >= 0; i--) {
                new_dim_vec.clear();
                InitValAST *initval = reinterpret_cast<InitValAST *>
                                                ((*initvalvec)[i].get());
                if(initval->type == InitValType::InitVal_Vec ||
                  initval->type == InitValType::InitVal_Null) {
                    if(elem_num == 0) {
                        new_dim_vec = dim_vec;
                        new_dim_vec.pop_back();
                    }
                    else {
                        int layer = 0;
                        int layer_size = 1;
                        while(layer < dim_vec_size) {
                            if(elem_num % layer_size == 0) {
                                new_dim_vec.push_back(dim_vec[layer]);
                                layer_size *= dim_vec[layer];
                                layer++;
                            }
                            else break;
                        }
                        new_dim_vec.pop_back();
                    }
                    if(new_dim_vec.empty()) {
                        std::cerr << "Error: Invalid Array.\n";
                        assert(false);
                    }
                    std::unique_ptr<std::vector<int> > new_arr = initval->getArrInit(new_dim_vec, str);
                    arr_ptr->insert(arr_ptr->end(), new_arr->begin(), new_arr->end());
                    elem_num = arr_ptr->size();
                }
                else if(initval->type == InitValType::InitVal_Exp) {
                    if(field == Field_Local) {
                        str += initval->DumpIR();
                        arr_ptr->push_back(ast_i - 1);
                    }
                    else {
                        arr_ptr->push_back(initval->ConstCalc());
                    }
                    elem_num++;
                    continue;
                }
            }
        }
        else if(type == InitVal_Exp) {
            std::cerr << "Error: Invalid ConstInitVal.\n";
            assert(false);
        }
        if(elem_num < arr_size) {
            if(field == Field_Local)
                for(int _ = elem_num; _ < arr_size; ++_) arr_ptr->push_back(-1);
            else
                for(int _ = elem_num; _ < arr_size; ++_) arr_ptr->push_back(0);
        }
        return arr_ptr; 
    }
};