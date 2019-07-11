/* Includes Source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/
#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>

#include "ast.h"
#include "utils.h"

#include "llvm-c/Core.h"

LLVMValueRef emit_evaluate_expression(LLVMBuilderRef builder, struct ast *a);
LLVMModuleRef emit_standard(struct ast *a);

void emit_manifest_ttl(LLVMModuleRef mod, struct ast *a, struct plugin_filenames *names);
void emit_plugin_ttl(LLVMModuleRef mod, struct ast *a, struct plugin_filenames *names);
void emit_x(LLVMBuilderRef builder, struct ast *a);

void finish_descriptor(LLVMValueRef uri);

// defined in codegen_std
extern LLVMContextRef global_context;
extern FILE *trace_file;

#endif
