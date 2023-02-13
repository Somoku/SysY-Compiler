#pragma once

#include "koopa.h"

koopa_raw_program_t generate_raw(const char *str, koopa_raw_program_builder_t raw_builder);
koopa_raw_program_builder_t new_builder();
void delete_builder(koopa_raw_program_builder_t raw_builder);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice, int st_offset, int &st_id, bool RA_call);
void Visit(const koopa_raw_function_t &func, int &st_id);
void Visit(const koopa_raw_basic_block_t &bb, int st_offset, int &st_id, bool RA_call);
void Visit(const koopa_raw_value_t &value, int st_offset, int &st_id, bool RA_call);
void Visit(const koopa_raw_return_t &ret, int st_offset, bool RA_call);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary, int &st_id);
void Visit(const koopa_raw_store_t &store, int &st_id, int st_offset);
void Visit(const koopa_raw_global_alloc_t &global_alloc);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);
int Visit(const koopa_raw_load_t &load, int &st_id);
void Visit(const koopa_raw_call_t &call, int st_offset);
int get_S_num(const koopa_raw_slice_t &slice);
int get_S_num(const koopa_raw_value_t &value);
int get_RA_num(const koopa_raw_slice_t &slice, bool &RA_call);
int get_RA_num(const koopa_raw_value_t &value, bool &RA_call);