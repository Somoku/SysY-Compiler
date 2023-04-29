#include "koopa.h"
#include "raw.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <algorithm>

std::unordered_map<koopa_raw_value_t, int> stack_offset;

// 生成 raw program
koopa_raw_program_t generate_raw(const char *str, koopa_raw_program_builder_t builder) {
    // 解析字符串 str, 得到 Koopa IR 程序
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
    
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

  // 访问所有全局变量
  if(program.values.len != 0)
    std::cout << "\t.data\n";
  Visit(program.values, 0, st_id, false);
  // 访问所有函数
  Visit(program.funcs, 0, st_id, false);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice, int st_offset, int &st_id, bool RA_call) {
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
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), st_offset, st_id, RA_call);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr), st_offset, st_id, RA_call);
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func, int &st_id) {
  if(func->bbs.len == 0)
    return;
  // 执行一些其他的必要操作
  std::cout << "\t.text" << std::endl;
  std::cout << "\t.globl ";
  std::cout << (func->name + 1) << std::endl;
  std::cout << (func->name + 1) << ":\n";

  // 获取存在返回值的指令数
  unsigned int S_num = get_S_num(func->bbs);
  bool RA_call = false;
  unsigned int RA_num = get_RA_num(func->bbs, RA_call);
  st_id = RA_num * 4;
  unsigned int st_offset = (((S_num + RA_num + RA_call) * 4 + 15) / 16) * 16;
  if(st_offset != 0){
    if (st_offset <= 2048)
      std::cout << "\taddi sp, sp, -" << st_offset << std::endl;
    else{
      std::cout << "\tli t0, -" << st_offset << std::endl;
      std::cout << "\tadd sp, sp, t0\n";
    }
    if(RA_call) {
      if(st_offset - 4 <= 2047) {
        std::cout << "\taddi t6, sp, " << st_offset - 4 << std::endl;
      }
      else {
        std::cout << "\tli t6, " << st_offset - 4 << std::endl;
        std::cout << "\tadd t6, t6, sp\n";
      }
      std::cout << "\tsw ra, (t6)\n";
    }
  } 
  // 访问所有基本块
  Visit(func->bbs, st_offset, st_id, RA_call);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb, int st_offset, int &st_id, bool RA_call) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  std::cout << (bb->name + 1) << ":\n";
  Visit(bb->insts, st_offset, st_id, RA_call);
}

// 访问指令
void Visit(const koopa_raw_value_t &value, int st_offset, int &st_id, bool RA_call) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret, st_offset, RA_call);
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
    case KOOPA_RVT_STORE:
      Visit(kind.data.store, st_id, st_offset);
      break;
    case KOOPA_RVT_LOAD:
      stack_offset[value] = Visit(kind.data.load, st_id);
      break;
    case KOOPA_RVT_ALLOC:
      Visit(value, st_id);
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      std::cout << "\t.globl " << value->name + 1 << std::endl;
      std::cout << value->name + 1 << ":\n";
      Visit(kind.data.global_alloc);
      break;
    case KOOPA_RVT_BRANCH:
      Visit(kind.data.branch);
      break;
    case KOOPA_RVT_JUMP:
      Visit(kind.data.jump);
      break;
    case KOOPA_RVT_CALL:
      Visit(kind.data.call, st_offset);
      if(value->ty->tag != KOOPA_RTT_UNIT) {
        stack_offset[value] = st_id;
        std::cout << "\tli t6, " << st_id << std::endl;
        std::cout << "\tadd t6, t6, sp\n";
        std::cout << "\tsw a0, (t6)\n";
        st_id += 4;
      }
      break;
    case KOOPA_RVT_GET_ELEM_PTR:
      Visit(kind.data.get_elem_ptr, st_id);
      stack_offset[value] = st_id;
      st_id += 4;
      break;
    case KOOPA_RVT_GET_PTR:
      Visit(kind.data.get_ptr, st_id);
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
void Visit(const koopa_raw_return_t &ret, int st_offset, bool RA_call) {
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
  if(st_offset != 0){
    if(RA_call) {
      if(st_offset - 4 <= 2047) {
        std::cout << "\taddi t6, sp, " << st_offset - 4 << std::endl;
      }
      else {
        std::cout << "\tli t6, " << st_offset - 4 << std::endl;
        std::cout << "\tadd t6, t6, sp\n";
      }
      std::cout << "\tlw ra, (t6)\n";
    }
    if (st_offset <= 2047)
      std::cout << "\taddi sp, sp, " << st_offset << std::endl;
    else {
      std::cout << "\tli t0, " << st_offset << std::endl;
      std::cout << "\tadd sp, sp, t0\n";
    }
  }
  std::cout << "\tret\n\n";
}

// 访问 integer
void Visit(const koopa_raw_integer_t &integer) {
  std::cout << integer.value << std::endl;
}

// 访问 binary 运算指令
void Visit(const koopa_raw_binary_t &binary, int &st_id) {
  bool lhs_int = false;
  bool rhs_int = false;

  if(binary.lhs->kind.tag == KOOPA_RVT_INTEGER) { 
    lhs_int = true;
    if((koopa_raw_binary_op)binary.op != KOOPA_RBO_ADD)
      std::cout << "\tli t2, " << binary.lhs->kind.data.integer.value << std::endl;
  }
  else{
    std::cout << "\tli t4, " << stack_offset[binary.lhs] << std::endl; 
    std::cout << "\tadd t4, sp, t4\n";
    std::cout << "\tlw t2, (t4)\n";
  }
  if(binary.rhs->kind.tag == KOOPA_RVT_INTEGER) {
    rhs_int = true;
    if((koopa_raw_binary_op)binary.op != KOOPA_RBO_ADD)
      std::cout << "\tli t3, " << binary.rhs->kind.data.integer.value << std::endl;
  }
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
      if(lhs_int && rhs_int && binary.lhs->kind.data.integer.value == 0)
        std::cout << "\tli t4, " << binary.rhs->kind.data.integer.value << std::endl;
      else {
        if(lhs_int)
          std::cout << "\tli t2, " << binary.lhs->kind.data.integer.value << std::endl;
        if(rhs_int)
          std::cout << "\tli t3, " << binary.rhs->kind.data.integer.value << std::endl;
        std::cout << "\tadd t4, t2, t3\n";
      }
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

// 访问 store 指令
void Visit(const koopa_raw_store_t &store, int &st_id, int st_offset) {
  if(store.value->kind.tag == KOOPA_RVT_INTEGER)
    std::cout << "\tli t0, " << store.value->kind.data.integer.value << std::endl;
  else if(store.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
    if(store.value->kind.data.func_arg_ref.index < 8)
      std::cout << "\tmv t0, a" << store.value->kind.data.func_arg_ref.index << std::endl;
    else {
      std::cout << "\tli t6, " << (store.value->kind.data.func_arg_ref.index - 8) * 4 + st_offset << std::endl;
      std::cout << "\tadd t6, t6, sp\n";
      std::cout << "\tlw t0, (t6)\n";
    }
  }
  else {
    std::cout << "\tli t6, " << stack_offset[store.value] << std::endl;
    std::cout << "\tadd t6, sp, t6\n";
    std::cout << "\tlw t0, (t6)\n";
  }
  if(store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    std::cout << "\tla t6, " << store.dest->name + 1 << std::endl;
    std::cout << "\tsw t0, (t6)\n";
  }
  else if(store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
          store.dest->kind.tag == KOOPA_RVT_GET_PTR) {
    if(stack_offset[store.dest] == 0)
      std::cout << "\tlw t6, (sp)\n";
    else {
      std::cout << "\tli t6, " << stack_offset[store.dest] << std::endl;
      std::cout << "\tadd t6, t6, sp\n";
      std::cout << "\tlw t5, (t6)\n";
    }
    std::cout << "\tsw t0, (t5)\n";
  }
  else {
    if(stack_offset.find(store.dest) == stack_offset.end()) {
      stack_offset[store.dest] = st_id;
      st_id += 4;
    }
    std::cout << "\tli t6, " << stack_offset[store.dest] << std::endl;
    std::cout << "\tadd t6, sp, t6\n";
    std::cout << "\tsw t0, (t6)\n";
  }
}

// 访问 load 指令
int Visit(const koopa_raw_load_t &load, int &st_id) {
  if(load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    std::cout << "\tla t0, " << load.src->name + 1 << std::endl;
    std::cout << "\tlw t1, 0(t0)\n";
    std::cout << "\tli t6, " << st_id << std::endl;
    std::cout << "\tadd t6, t6, sp\n";
    std::cout << "\tsw t1, (t6)\n";
    st_id += 4;
    return st_id - 4;
  }
  else if(load.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR ||
          load.src->kind.tag == KOOPA_RVT_GET_PTR) {
    if(stack_offset[load.src] == 0)
      std::cout << "\tlw t1, (sp)\n";
    else {
      std::cout << "\tli t6, " << stack_offset[load.src] << std::endl;
      std::cout << "\tadd t6, t6, sp\n";
      std::cout << "\tlw t1, (t6)\n";
    }
    std::cout << "\tlw t2, (t1)\n";
    std::cout << "\tli t6, " << st_id << std::endl;
    std::cout << "\tadd t6, t6, sp\n";
    std::cout << "\tsw t2, (t6)\n";
    st_id += 4;
    return st_id - 4;
  }
  return stack_offset[load.src];
}

// 访问 global alloc 指令
void Visit(const koopa_raw_global_alloc_t &global_alloc) {
  if(global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT)
    std::cout << "\t.zero " << calc_size(global_alloc.init->ty) << std::endl;
  else if(global_alloc.init->kind.tag == KOOPA_RVT_INTEGER)
    std::cout << "\t.word " << global_alloc.init->kind.data.integer.value << std::endl;
  else if(global_alloc.init->kind.tag == KOOPA_RVT_AGGREGATE)
    Visit(global_alloc.init->kind.data.aggregate);
  else {
    std::cerr << "Error: Unknown global allocation type.\n";
    assert(false);
  }
}

// 访问 aggregate, 获取初始化数据
void Visit(const koopa_raw_aggregate_t &aggregate) {
  for(size_t i = 0; i < aggregate.elems.len; ++i) {
    koopa_raw_value_t ptr = reinterpret_cast<koopa_raw_value_t>(aggregate.elems.buffer[i]);
    if(ptr->kind.tag == KOOPA_RVT_INTEGER)
      std::cout << "\t.word " << ptr->kind.data.integer.value << std::endl;
    else
      Visit(ptr->kind.data.aggregate);
  }
}

// 访问 branch 指令
void Visit(const koopa_raw_branch_t &branch) {
  if(branch.cond->kind.tag == KOOPA_RVT_INTEGER)
    std::cout << "\tli t0, " << branch.cond->kind.data.integer.value << std::endl;
  else {
    std::cout << "\tli t6, " << stack_offset[branch.cond] << std::endl;
    std::cout << "\tadd t6, sp, t6\n";
    std::cout << "\tlw t0, (t6)\n";
  }
  std::cout << "\tbnez t0, " << "median_branch" << (branch.true_bb->name + 1) << std::endl;
  std::cout << "\tbeqz t0, " << "median_branch" << (branch.false_bb->name + 1) << std::endl;
  std::cout << "median_branch" << (branch.true_bb->name + 1) << ":" << std::endl;
  std::cout << "\tj " << (branch.true_bb->name + 1) << std::endl;
  std::cout << "median_branch" << (branch.false_bb->name + 1) << ":" << std::endl;
  std::cout << "\tj " << (branch.false_bb->name + 1) << std::endl;
}

// 访问 jump 指令
void Visit(const koopa_raw_jump_t &jump) {
  std::cout << "\tj " << (jump.target->name + 1) << std::endl;
}

// 访问 call 指令
void Visit(const koopa_raw_call_t &call, int st_offset) {
  int param_id = 0;
  int param_len = call.args.len;
  for(int i = 0; i < param_len; ++i) {
    koopa_raw_value_t ptr = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);
    if(ptr->kind.tag == KOOPA_RVT_INTEGER) {
      if(i < 8) {
        std::cout << "\tli a" << i << ", " << ptr->kind.data.integer.value << std::endl;
      }
      else {
        std::cout << "\tli t0, " << ptr->kind.data.integer.value << std::endl;
        std::cout << "\tli t6, " << param_id << std::endl;
        std::cout << "\tadd t6, t6, sp\n";
        std::cout << "\tsw t0, (t6)\n";
        param_id += 4;
      }
    }
    else {
      if(stack_offset.find(ptr) == stack_offset.end()) {
        std::cerr << "Error: Invalid parameter.\n";
      }
      else {
        if(i < 8) {
          std::cout << "\tli t6, " << stack_offset[ptr] << std::endl;
          std::cout << "\tadd t6, t6, sp\n";
          std::cout << "\tlw a" << i << ", (t6)\n";
        }
        else {
          std::cout << "\tli t6, " << stack_offset[ptr] << std::endl;
          std::cout << "\tadd t6, t6, sp\n";
          std::cout << "\tlw t0, (t6)\n";
          std::cout << "\tli t5, " << param_id << std::endl;
          std::cout << "\tadd t5, t5, sp\n";
          std::cout << "\tsw t0, (t5)\n";
          param_id += 4;
        }  
      }
    }
  }
  std::cout << "\tcall " << call.callee->name + 1 << std::endl;
}

// 访问 get_elem_ptr 指令
void Visit(const koopa_raw_get_elem_ptr_t &get_elem_ptr, int &st_id) {
  size_t off_size;
  if(get_elem_ptr.src->ty->data.pointer.base->tag == KOOPA_RTT_INT32)
    off_size = calc_size(get_elem_ptr.src->ty->data.pointer.base);
  else
    off_size = calc_size(get_elem_ptr.src->ty->data.pointer.base) / (get_elem_ptr.src->ty->data.pointer.base->data.array.len);
  if(get_elem_ptr.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    std::cout << "\tla t6, " << get_elem_ptr.src->name + 1 << std::endl;
  else if(get_elem_ptr.src->kind.tag == KOOPA_RVT_ALLOC) {
    std::cout << "\tli t6, " << stack_offset[get_elem_ptr.src] << std::endl;
    std::cout << "\tadd t6, t6, sp\n";
  }
  else {
    std::cout << "\tli t5, " << stack_offset[get_elem_ptr.src] << std::endl;
    std::cout << "\tadd t5, t5, sp\n";
    std::cout << "\tlw t6, (t5)\n";
  }

  if(get_elem_ptr.index->kind.tag == KOOPA_RVT_INTEGER) {
    if(get_elem_ptr.index->kind.data.integer.value == 0) {
      std::cout << "\tli t1, " << st_id << std::endl;
      std::cout << "\tadd t1, t1, sp\n";
      std::cout << "\tsw t6, (t1)\n";
      return;
    }
    else
      std::cout << "\tli t1, " << get_elem_ptr.index->kind.data.integer.value << std::endl;
  }
  else {
    std::cout << "\tli t5, " << stack_offset[get_elem_ptr.index] << std::endl;
    std::cout << "\tadd t5, t5, sp\n";
    std::cout << "\tlw t1, (t5)\n";
  }
  std::cout << "\tli t2, " << off_size << std::endl;
  std::cout << "\tmul t1, t1, t2\n";
  std::cout << "\tadd t6, t6, t1\n";
  std::cout << "\tli t1, " << st_id << std::endl;
  std::cout << "\tadd t1, t1, sp\n";
  std::cout << "\tsw t6, (t1)\n";
}

// 访问 get_ptr 指令
void Visit(const koopa_raw_get_ptr_t &get_ptr, int &st_id) {
  size_t off_size = 0;
  if(get_ptr.src->ty->data.pointer.base->tag == KOOPA_RTT_INT32)
    off_size = calc_size(get_ptr.src->ty->data.pointer.base);
  else
    off_size = calc_size(get_ptr.src->ty->data.pointer.base);

  if(get_ptr.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    std::cout << "\tla t6, " << get_ptr.src->name + 1 << std::endl;
  else if(get_ptr.src->kind.tag == KOOPA_RVT_ALLOC) {
    std::cout << "\tli t6, " << stack_offset[get_ptr.src] << std::endl;
    std::cout << "\tadd t6, t6, sp\n";
  }
  else {
    std::cout << "\tli t5, " << stack_offset[get_ptr.src] << std::endl;
    std::cout << "\tadd t5, t5, sp\n";
    std::cout << "\tlw t6, (t5)\n";
  }

  if(get_ptr.index->kind.tag == KOOPA_RVT_INTEGER) {
    if(get_ptr.index->kind.data.integer.value == 0) {
      std::cout << "\tli t1, " << st_id << std::endl;
      std::cout << "\tadd t1, t1, sp\n";
      std::cout << "\tsw t6, (t1)\n";
      return;
    }
    else
      std::cout << "\tli t1, " << get_ptr.index->kind.data.integer.value << std::endl;
  }
  else {
    std::cout << "\tli t5, " << stack_offset[get_ptr.index] << std::endl;
    std::cout << "\tadd t5, t5, sp\n";
    std::cout << "\tlw t1, (t5)\n";
  }
  std::cout << "\tli t2, " << off_size << std::endl;
  std::cout << "\tmul t1, t1, t2\n";
  std::cout << "\tadd t6, t6, t1\n";
  std::cout << "\tli t1, " << st_id << std::endl;
  std::cout << "\tadd t1, t1, sp\n";
  std::cout << "\tsw t6, (t1)\n";
}

// 访问 alloc 指令
void Visit(const koopa_raw_value_t &value, int &st_id) {
  koopa_raw_type_kind_t *base;
  int arr_size;
  switch(value->ty->data.pointer.base->tag) {
    case KOOPA_RTT_INT32:
      stack_offset[value] = st_id;
      st_id += 4;
      break;
    case KOOPA_RTT_ARRAY:
      base = (koopa_raw_type_kind_t *)(value->ty->data.pointer.base);
      arr_size = 4;
      while(base->tag != KOOPA_RTT_INT32) {
        arr_size *= base->data.array.len;
        base = (koopa_raw_type_kind_t *)(base->data.array.base);
      }
      stack_offset[value] = st_id;
      st_id += arr_size;
      break;
    case KOOPA_RTT_POINTER:
      stack_offset[value] = st_id;
      st_id += 4;
      break;
    default:
      assert(false);
  }
}

// 访问 raw slice, 获取存在返回值的指令个数
unsigned int get_S_num(const koopa_raw_slice_t &slice) {
  unsigned int S_num = 0;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        S_num += get_S_num(reinterpret_cast<koopa_raw_basic_block_t>(ptr)->insts);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        S_num += get_S_num(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
  return S_num;
}

// 访问指令，若该指令存在返回值，则返回 1, 否则返回 0
unsigned int get_S_num(const koopa_raw_value_t &value) {
  const auto &ty = value->ty;
  koopa_raw_type_kind_t *base;
  unsigned int arr_size;
  if(ty->tag == KOOPA_RTT_UNIT)
    return 0;
  if(value->kind.tag == KOOPA_RVT_ALLOC) {
    if(ty->data.pointer.base->tag == KOOPA_RTT_ARRAY) {
        base = (koopa_raw_type_kind_t *)(ty->data.pointer.base);
        arr_size = 4;
        while(base->tag != KOOPA_RTT_INT32) {
          arr_size *= base->data.array.len;
          base = (koopa_raw_type_kind_t *)(base->data.array.base);
        }
        return arr_size / 4;
    }
    else
      return 1;
  }
  return 1;
}

// 访问 raw slice, 获取需要分配栈空间的参数数量以及是否要为 ra 分配空间
unsigned int get_RA_num(const koopa_raw_slice_t &slice, bool &RA_call) {
  unsigned int RA_num = 0;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch(slice.kind) {
      case KOOPA_RSIK_BASIC_BLOCK:
        RA_num = std::max(RA_num, get_RA_num(reinterpret_cast<koopa_raw_basic_block_t>(ptr)->insts, RA_call));
        break;
      case KOOPA_RSIK_VALUE:
        RA_num = std::max(RA_num, get_RA_num(reinterpret_cast<koopa_raw_value_t>(ptr), RA_call));
        break;
      default:
        assert(false);
    }
  }
  return RA_num;
}

// 访问指令，若指令为 call 指令，则返回参数个数 - 8，并设 RA_call 为 true
unsigned int get_RA_num(const koopa_raw_value_t &value, bool &RA_call) {
  if(value->kind.tag != KOOPA_RVT_CALL)
    return 0;
  RA_call = true;
  unsigned int param_num = value->kind.data.call.args.len;
  if(param_num <= 8)
    return 0;
  return param_num - 8;
}

// 计算 zeroinit 的数据大小
size_t calc_size(const koopa_raw_type_t &ty) {
  if(ty->tag == KOOPA_RTT_INT32)
    return 4;
  return (ty->data.array.len) * calc_size(ty->data.array.base);
}