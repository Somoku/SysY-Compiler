#include "symbol.hpp"

symbol_table_list_elem_t *curr_symbol_table = nullptr;

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