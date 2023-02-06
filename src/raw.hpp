#pragma once

#include "koopa.h"

koopa_raw_program_t generate_raw(const char *str, koopa_raw_program_builder_t raw_builder);
koopa_raw_program_builder_t new_builder();
void delete_builder(koopa_raw_program_builder_t raw_builder);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice, int st_offset, int &st_id);
void Visit(const koopa_raw_function_t &func, int &st_id);
void Visit(const koopa_raw_basic_block_t &bb, int st_offset, int &st_id);
void Visit(const koopa_raw_value_t &value, int st_offset, int &st_id);
void Visit(const koopa_raw_return_t &ret, int st_offset);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary, int &st_id);
void Visit(const koopa_raw_store_t &store, int &st_id);
int Visit(const koopa_raw_load_t &load);
int get_st_num(const koopa_raw_slice_t &slice);
int get_st_num(const koopa_raw_value_t &value);