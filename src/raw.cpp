#include "koopa.h"
#include "raw.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>

std::unordered_map<koopa_raw_value_t, int> stack_offset;

// 生成 raw program
koopa_raw_program_t generate_raw(const char *str, koopa_raw_program_builder_t builder) {
    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
    
    // 创建一个 raw program builder, 用来构建 raw program
    // koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    
    // 将 Koopa IR 程序转换为 raw program
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    
    // 释放 Koopa IR 程序占用的内存
    koopa_delete_program(program);

    // 返回 raw program
    return raw;
}

// 生成 raw program builder
koopa_raw_program_builder_t new_builder() {
    return koopa_new_raw_program_builder();
}

// 释放 raw program builder
void delete_builder(koopa_raw_program_builder_t builder) {
    koopa_delete_raw_program_builder(builder);
}

// 处理 raw program
void Visit(const koopa_raw_program_t &program) {
  int st_id = 0;
  // 执行一些其他的必要操作
  std::cout << "\t.text" << std::endl;

  // 访问所有全局变量
  Visit(program.values, 0, st_id);
  // 访问所有函数
  Visit(program.funcs, 0, st_id);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice, int st_offset, int &st_id) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr), st_id);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), st_offset, st_id);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr), st_offset, st_id);
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func, int &st_id) {
  // 执行一些其他的必要操作
  std::cout << "\t.globl ";
  std::cout << (func->name + 1) << std::endl;
  std::cout << (func->name + 1) << ":\n";

  // 获取存在返回值的指令数
  int st_num = get_st_num(func->bbs);
  int st_offset = st_num * 4;
  if (st_offset <= 2048)
    std::cout << "\taddi sp, sp, -" << st_offset << std::endl;
  else{
    std::cout << "\tli t0, -" << st_offset << std::endl;
    std::cout << "\tadd sp, sp, t0\n";
  }
  // 访问所有基本块
  Visit(func->bbs, st_offset, st_id);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb, int st_offset, int &st_id) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts, st_offset, st_id);
}

// 访问指令
void Visit(const koopa_raw_value_t &value, int st_offset, int &st_id) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  // std::cout << "# kind.tag = " << kind.tag << std::endl;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret, st_offset);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      Visit(kind.data.binary, st_id);
      stack_offset[value] = st_id;
      st_id += 4;
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

// 访问 return 指令
void Visit(const koopa_raw_return_t &ret, int st_offset) {
  if(!ret.value) 
    std::cout << "\tli a0, 0\n";
  else {
    const auto &kind = ret.value->kind;
    if (kind.tag == KOOPA_RVT_INTEGER)
      std::cout << "\tli a0, " << kind.data.integer.value << std::endl;
    else {
      std::cout << "\tli t1, " << stack_offset[ret.value] << std::endl;
      std::cout << "\tadd t1, t1, sp\n";
      std::cout << "\tlw a0, (t1)\n";
    }
  }
  // Visit(ret.value, st_num);
  if (st_offset <= 2047)
    std::cout << "\taddi sp, sp, " << st_offset << std::endl;
  else {
    std::cout << "\tli t0, " << st_offset << std::endl;
    std::cout << "\tadd sp, sp, t0\n";
  }
  std::cout << "\tret" << std::endl;
}

// 访问 integer
void Visit(const koopa_raw_integer_t &integer) {
  std::cout << integer.value << std::endl;
}

// 访问 binary 运算指令
void Visit(const koopa_raw_binary_t &binary, int &st_id) {
  if(binary.lhs->kind.tag == KOOPA_RVT_INTEGER)
    std::cout << "\tli t2, " << binary.lhs->kind.data.integer.value << std::endl;
  else{
    std::cout << "\tli t4, " << stack_offset[binary.lhs] << std::endl; 
    std::cout << "\tadd t4, sp, t4\n";
    std::cout << "\tlw t2, (t4)\n";
  }
  if(binary.rhs->kind.tag == KOOPA_RVT_INTEGER)
    std::cout << "\tli t3, " << binary.rhs->kind.data.integer.value << std::endl;
  else{
    std::cout << "\tli t4, " << stack_offset[binary.rhs] << std::endl; 
    std::cout << "\tadd t4, sp, t4\n";
    std::cout << "\tlw t3, (t4)\n";
  }

  switch((koopa_raw_binary_op)binary.op) {
    case KOOPA_RBO_NOT_EQ:
      std::cout << "\txor t4, t2, t3\n";
      std::cout << "\tsnez t4, t4\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_EQ:
      std::cout << "\txor t4, t2, t3\n";
      std::cout << "\tseqz t4, t4\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_GT:
      std::cout << "\tsgt t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_LT:
      std::cout << "\tslt t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_GE:
      std::cout << "\tslt t4, t2, t3\n";
      std::cout << "\tseqz t4, t4\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_LE:
      std::cout << "\tsgt t4, t2, t3\n";
      std::cout << "\tseqz t4, t4\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_ADD:
      std::cout << "\tadd t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_SUB:
      std::cout << "\tsub t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_MUL:
      std::cout << "\tmul t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_DIV:
      std::cout << "\tdiv t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_MOD:
      std::cout << "\trem t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_AND:
      std::cout << "\tand t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    case KOOPA_RBO_OR:
      std::cout << "\tor t4, t2, t3\n";
      std::cout << "\tli t6, " << st_id << std::endl;
      std::cout << "\tadd t6, sp, t6\n";
      std::cout << "\tsw t4, (t6)\n";
      break;
    default:
      assert(false);
  }
}

// 访问 raw slice, 获取存在返回值的指令个数
int get_st_num(const koopa_raw_slice_t &slice) {
  int st_num = 0;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        st_num += get_st_num(reinterpret_cast<koopa_raw_basic_block_t>(ptr)->insts);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        st_num += get_st_num(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
  return st_num;
}

// 访问指令，若该指令存在返回值，则返回 1, 否则返回 0
int get_st_num(const koopa_raw_value_t &value) {
  const auto &ty = value->ty;
  if(ty->tag == KOOPA_RTT_UNIT)
    return 0;
  return 1;
}