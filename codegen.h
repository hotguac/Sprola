/* Includes Source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/
#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

#include "llvm-c/Core.h"

void emit_x(LLVMBuilderRef builder, struct ast *a);
LLVMValueRef emit_evaluate_expression(LLVMBuilderRef builder, struct ast *a);

#endif
