#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

enum symbol_field {
    Field_Global,
    Field_Local
};

enum symbol_tag {
    Symbol_Const,
    Symbol_Var,
    Symbol_Func,
    Symbol_Arr,
    Symbol_Pointer
};

struct symbol_t {
    int value;
    symbol_tag tag;
};

static int global_symbol_table_num = 0;

struct symbol_table_elem_t {
    std::unique_ptr<std::unordered_map<std::string, symbol_t> > symbol_table_elem_ptr;
    int symbol_table_num;

    symbol_table_elem_t() {
        symbol_table_elem_ptr = std::unique_ptr<std::unordered_map<std::string, symbol_t> >
                                (new std::unordered_map<std::string, symbol_t>());
        symbol_table_num = global_symbol_table_num++;
    };
};

struct symbol_table_list_elem_t {
    std::unique_ptr<symbol_table_elem_t> symbol_table_ptr;
    symbol_table_list_elem_t *prev_elem;
    symbol_table_list_elem_t *next_elem;

    symbol_table_list_elem_t() : prev_elem(nullptr), next_elem(nullptr) {
        symbol_table_ptr = std::unique_ptr<symbol_table_elem_t>(new symbol_table_elem_t());
    };    
};

void create_symbol_table();
void delete_symbol_table();
symbol_table_list_elem_t *search_symbol_table(std::string ident);
std::string koopa_lib();