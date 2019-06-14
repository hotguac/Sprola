# LLVM C API Example Calls and Generated IR

This document has code snippets that can be used as templates
for code generation routines. The first section shows the IR code and then the C API calls that will generate that IR.

## IR code with C API calls

*Example 1 - the alloca statement*

%1 = alloca i8*, align 8

*The C API*

LLVMBuilderRef builder = LLVMCreateBuilder();
LLVMTypeRef void_ptr = LLVMPointerType(LLVMInt8Type(), 0);
LLVMValueRef x = LLVMBuildAlloca(builder, void_ptr, "");
LLVMSetAlignment(x, 8);
LLVMDisposeBuilder(builder);
