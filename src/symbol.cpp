#include "symbol.hpp"

symbol_table_list_elem_t *curr_symbol_table = nullptr;
std::unordered_map<std::string, symbol_t> global_symbol_table;

void create_symbol_table() {
    if(curr_symbol_table == nullptr) {
        curr_symbol_table = new symbol_table_list_elem_t();
        return;
    }
    symbol_table_list_elem_t *new_symbol_table = new symbol_table_list_elem_t();
    new_symbol_table->prev_elem = curr_symbol_table;
    curr_symbol_table->next_elem = new_symbol_table;
    curr_symbol_table = new_symbol_table;
}

void delete_symbol_table() {
    if(curr_symbol_table == nullptr) {
        std::cerr << "Error: No symbol table to delete." << std::endl;
        return;
    }
    symbol_table_list_elem_t *new_curr_symbol_table = curr_symbol_table->prev_elem;
    delete curr_symbol_table;
    curr_symbol_table = new_curr_symbol_table;
    if(curr_symbol_table != nullptr)
        curr_symbol_table->next_elem = nullptr;
}

symbol_table_list_elem_t *search_symbol_table(std::string ident) {
    symbol_table_list_elem_t *symbol_table_list_elem = curr_symbol_table;
    while(symbol_table_list_elem != nullptr) {
        if((*(symbol_table_list_elem->symbol_table_ptr->symbol_table_elem_ptr)).find(ident) == 
            (*(symbol_table_list_elem->symbol_table_ptr->symbol_table_elem_ptr)).end()) {
            symbol_table_list_elem = symbol_table_list_elem->prev_elem;
            continue;
        }
        return symbol_table_list_elem;
    }
    return nullptr;
}

std::string koopa_lib() {
  std::string str;
  str += "decl @getint(): i32\n";
  str += "decl @getch(): i32\n";
  str += "decl @getarray(*i32): i32\n";
  str += "decl @putint(i32)\n";
  str += "decl @putch(i32)\n";
  str += "decl @putarray(i32, *i32)\n";
  str += "decl @starttime()\n";
  str += "decl @stoptime()\n";
  global_symbol_table[std::string("getint")] = symbol_t{0, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("getch")] = symbol_t{0, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("getarray")] = symbol_t{0, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("putint")] = symbol_t{1, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("putch")] = symbol_t{1, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("putarray")] = symbol_t{1, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("starttime")] = symbol_t{1, symbol_tag::Symbol_Func};
  global_symbol_table[std::string("stoptime")] = symbol_t{1, symbol_tag::Symbol_Func};
  return str;
}