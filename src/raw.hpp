#pragma once

#include "koopa.h"

koopa_raw_program_t generate_raw(const char *str, koopa_raw_program_builder_t raw_builder);
koopa_raw_program_builder_t new_builder();
void delete_builder(koopa_raw_program_builder_t raw_builder);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
