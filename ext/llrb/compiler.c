/*
 * compiler.c: Compiles encoded YARV instructions structured as Control Flow Graph to LLVM IR.
 */

#include <stdbool.h>
#include <string.h>
#include "llvm-c/BitReader.h"
#include "llvm-c/Core.h"
#include "cruby.h"
#include "funcs.h"

// Store compiler's internal state and shared variables
struct llrb_compiler {
  const struct rb_iseq_constant_body *body;
  LLVMValueRef func;
  LLVMBuilderRef builder;
  LLVMModuleRef mod;
};

static LLVMValueRef
llrb_value(VALUE value)
{
  return LLVMConstInt(LLVMInt64Type(), value, false); // TODO: support 32bit for VALUE type
}

static inline LLVMValueRef
llrb_get_cfp(struct llrb_compiler *c)
{
  return LLVMGetParam(c->func, 1);
}

static LLVMValueRef
llrb_plus(struct llrb_compiler *c, LLVMValueRef lhs, LLVMValueRef rhs)
{
  LLVMValueRef args[] = { lhs, rhs };
  return LLVMBuildCall(c->builder, llrb_get_function(c->mod, "llrb_insn_opt_plus"), args, 2, "");
}

static void
llrb_compile_prototype(struct llrb_compiler *c, LLVMModuleRef mod)
{
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(c->func, "entry");
  LLVMPositionBuilderAtEnd(c->builder, block);

  LLVMValueRef v1 = llrb_plus(c, llrb_value(INT2FIX(1)), llrb_value(INT2FIX(2)));
  LLVMValueRef v2 = llrb_plus(c, v1, llrb_value(INT2FIX(3)));
  LLVMValueRef v3 = llrb_plus(c, v2, llrb_value(INT2FIX(4)));
  LLVMValueRef v4 = llrb_plus(c, v3, llrb_value(INT2FIX(5)));

  LLVMValueRef args[] = { llrb_get_cfp(c), v4 };
  LLVMBuildCall(c->builder, llrb_get_function(c->mod, "llrb_push_result"), args, 2, "");
  LLVMBuildRet(c->builder, llrb_get_cfp(c));
}

static LLVMModuleRef
llrb_build_initial_module()
{
  LLVMMemoryBufferRef buf;
  char *err;
  if (LLVMCreateMemoryBufferWithContentsOfFile("ext/insns.bc", &buf, &err)) {
    rb_raise(rb_eCompileError, "LLVMCreateMemoryBufferWithContentsOfFile Error: %s", err);
  }

  LLVMModuleRef ret;
  if (LLVMParseBitcode2(buf, &ret)) {
    rb_raise(rb_eCompileError, "LLVMParseBitcode2 Failed!");
  }
  LLVMDisposeMemoryBuffer(buf);
  return ret; // LLVMModuleCreateWithName("llrb");
}

// Compiles Control Flow Graph having encoded YARV instructions to LLVM IR.
static LLVMValueRef
llrb_compile_cfg(LLVMModuleRef mod, const rb_iseq_t *iseq, const char* funcname)
{
  LLVMTypeRef args[] = { LLVMInt64Type(), LLVMInt64Type() };
  LLVMValueRef func = LLVMAddFunction(mod, funcname,
      LLVMFunctionType(LLVMInt64Type(), args, 2, false));

  struct llrb_compiler compiler = (struct llrb_compiler){
    .body = iseq->body,
    .func = func,
    .builder = LLVMCreateBuilder(),
    .mod = mod,
  };
  llrb_compile_prototype(&compiler, mod);

  return func;
}

// In this function, LLRB has following dependency tree without mutual dependencies:
// llrb.c -> compiler.c -> parser.c, optimizer.cc
//
// llrb_create_native_func() uses a LLVM function named as `funcname` defined in returned LLVM module.
LLVMModuleRef
llrb_compile_iseq(const rb_iseq_t *iseq, const char* funcname)
{
  extern void llrb_parse_iseq(const rb_iseq_t *iseq);
  llrb_parse_iseq(iseq);

  LLVMModuleRef mod = llrb_build_initial_module();
  LLVMValueRef func = llrb_compile_cfg(mod, iseq, funcname);

  extern void llrb_optimize_function(LLVMModuleRef cmod, LLVMValueRef cfunc);
  llrb_optimize_function(mod, func);

  //LLVMDumpModule(mod);
  return mod;
}

void
Init_compiler(VALUE rb_mJIT)
{
  rb_eCompileError = rb_define_class_under(rb_mJIT, "CompileError", rb_eStandardError);
}
