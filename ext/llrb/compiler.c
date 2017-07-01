#include <stdbool.h>
#include <string.h>
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

static void
llrb_compile_prototype(struct llrb_compiler *c, LLVMModuleRef mod)
{
  LLVMBasicBlockRef block = LLVMAppendBasicBlock(c->func, "entry");
  LLVMPositionBuilderAtEnd(c->builder, block);

  LLVMValueRef args[] = { llrb_get_cfp(c), llrb_value(INT2FIX(15)) };
  LLVMBuildCall(c->builder, llrb_get_function(c->mod, "llrb_push_result"), args, 2, "");
  LLVMBuildRet(c->builder, llrb_get_cfp(c));
}

void llrb_optimize_function(LLVMModuleRef cmod, LLVMValueRef cfunc);

LLVMModuleRef
llrb_compile_iseq(const rb_iseq_t *iseq, const char* funcname)
{
  LLVMModuleRef mod = LLVMModuleCreateWithName("llrb");

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
  llrb_optimize_function(mod, func);
  return mod;
}

void
Init_compiler(VALUE rb_mJIT)
{
  rb_eCompileError = rb_define_class_under(rb_mJIT, "CompileError", rb_eStandardError);
}
