#include "llvm-c/Core.h"

#include "codegen.h"
#include "sprola.h"
#include "symbols.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(char const*);

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) verify function
/*----------------------------------------------------------------------------*/
void emit_var_declaration(LLVMBuilderRef builder, struct ast *a)
{
  if (a == NULL) {
    return;
  }

  if (verbose_flag) {
    fprintf(stderr, "emit_var_declaration\n");
  }

  struct var_decl *v = (struct var_decl *) a;
  struct symref *sr = (struct symref *) v->sym;
  struct symbol *s = sr->sym;

  if (verbose_flag) {
    //fprintf(stderr, "name = %s\n", s->name);
    //fprintf(stderr, "family = %d\n", s->sym_family);
    //fprintf(stderr, "type = %s\n", s->sym_typ);
  }

  if (s->sym_family != VARIABLE_NAME) {
    fprintf(stderr, "Error - emit_var_declaration - expecting variable declaration\n");
    fprintf(stderr, " - found type %d\n", s->sym_family);
  }

  LLVMTypeRef this_type = NULL;

  if (!strcmp(s->sym_typ, "int")) {
    this_type = LLVMInt32Type();
  } else if (!strcmp(s->sym_typ, "float")) {
    this_type = LLVMFloatType();
  } else {
    fprintf(stderr, "Error - emit_var_declaration - unknown type %s\n", s->sym_typ);
  }

  s->value = LLVMBuildAlloca(builder, this_type, s->name);
  LLVMSetAlignment(s->value, 4);

}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_evaluate_condition(LLVMBuilderRef builder, struct ast *a)
{
  LLVMValueRef result = NULL;

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_evaluate_condition\n");
  }

  struct condition *c = (struct condition *) a;

  LLVMValueRef left = emit_evaluate_expression(builder, c->left);
  LLVMValueRef right = emit_evaluate_expression(builder, c->right);

  switch (c->operator) {
    case L_greater_than:
      result = LLVMBuildICmp(builder, LLVMIntSGT, left, right, "");
      break;
    case L_less_than:
      result = LLVMBuildICmp(builder, LLVMIntSLT, left, right, "");
      break;
    case L_equals:
      result = LLVMBuildICmp(builder, LLVMIntEQ, left, right, "");
      break;
    default:
      fprintf(stderr, "Error - emit_evaluate_condition unknown condition %d\n", c->operator);
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_evaluate_expression(LLVMBuilderRef builder, struct ast *a)
{
  struct intval *i;
  struct symref *ref;
  struct symbol *sym;

  //fprintf(stderr, "****in process - emit_evaluate_expression\n");

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_evaluate_expression\n");
    exit(1);
  }

  switch (a->nodetype) {
    case N_condition:
      //fprintf(stderr, "work on next - N_condition\n");
      return emit_evaluate_condition(builder,a);
      break;
    case N_integer:
      //fprintf(stderr, "!!in process - N_integer\n");
      i = (struct intval *)a;
      return LLVMConstInt(LLVMInt32Type(), i->number, 0);
      break;
    case N_float:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_float\n");
      break;
    case N_symbol_ref:
      //fprintf(stderr, "inprocess - N_symbol_ref\n");
      ref = (struct symref *) a;
      sym = ref->sym;
      //fprintf(stderr, "eee id = %s , value = %s\n", sym->name, LLVMPrintValueToString(sym->value));
      //TODO(jkokosa) - always assume a symbol ref is an Alloca and needs loaded
      return LLVMBuildLoad(builder, sym->value, "");
      //return sym->value;
      break;
    case N_array_ref:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_array_ref\n");
      break;
    case N_add:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_add\n");
      LLVMValueRef left = emit_evaluate_expression(builder, a->l);
      LLVMValueRef right = emit_evaluate_expression(builder, a->r);
      //fprintf(stderr, "before LLVMBuildAdd\n");
      //LLVMValueRef temp = LLVMBuildAdd(builder, left, right, "");
      return LLVMBuildAdd(builder, left, right, "");
      //fprintf(stderr, "after LLVMBuildAdd\n");
      //dump_current_function(builder);
      //return temp;
      break;
    case N_subtract:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_subtract\n");
      break;
    case N_multiply:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_multiply\n");
      break;
    case N_divide:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_divide\n");
      break;
    case N_negate:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_negate\n");
      break;
    default:
      fprintf(stderr, "****in process - emit_evaluate_expression - unexpected nodetype %d\n", a->nodetype);
  }

  // This is a dummy statement until function is ready
  return LLVMConstInt(LLVMInt1Type(), 0, 0);
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
LLVMValueRef get_target(LLVMBuilderRef builder, struct ast *a)
{
  struct symref *ref;
  struct symbol *sym;
  //fprintf(stderr, "in process - get_target\n");

  if (a == NULL) {
    fprintf(stderr, "Error - get_target - NULL\n");
    exit(1);
  }

  switch (a->nodetype) {
    case N_symbol_ref:
      //fprintf(stderr, "in get_target found node of symbol ref\n");
      ref = (struct symref *) a;
      sym = ref->sym;
      return sym->value;
    default:
      fprintf(stderr, "in get_target found unknown nodetype %d\n", a->nodetype);
      return NULL;
  }
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) verify function
/*----------------------------------------------------------------------------*/
void emit_assignment(LLVMBuilderRef builder, struct ast *a)
{
  //fprintf(stderr, "in process - emit_assignment\n");
  if (a == NULL) {
    fprintf(stderr, "Error - emit_assignment - NULL\n");
    exit(1);
  }

  if (a->nodetype != N_assignment) {
    fprintf(stderr, "Error - expected for assignment, found %d\n", a->nodetype);
    exit(1);
  }

  struct symasgn *assign = (struct symasgn *) a;

  LLVMValueRef value = emit_evaluate_expression(builder, assign->value);
  LLVMValueRef target = get_target(builder, assign->target);

  if (!target) {
    fprintf(stderr, "emit_assignment - null target\n");
    return; // don't know what to do yet
  }

  if (!value) {
    fprintf(stderr, "emit_assignment - null value\n");
    return; // don't know what to do yet
  }
  /*
  fprintf(stderr, "before emit assignment store\n");
  fprintf(stderr, "value = %s\n", LLVMPrintValueToString(value));
  fprintf(stderr, "target = %s\n", LLVMPrintValueToString(target));
  */
  LLVMBuildStore(builder, value, target);
  //LLVMValueRef x = LLVMBuildStore(builder, value, target);
  /*
  fprintf(stderr, "x = %s\n", LLVMPrintValueToString(x));
  fprintf(stderr, "------------------------------------\n");
  fprintf(stderr, "%s\n", LLVMPrintValueToString(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder))));
  fprintf(stderr, "------------------------------------\n");
  fprintf(stderr, "after emit assignment store\n\n");
  */
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
void emit_code_block(LLVMBuilderRef builder, LLVMBasicBlockRef bb, struct ast *a, LLVMBasicBlockRef next)
{
  fprintf(stderr, "in process - emit_code_block\n");
  LLVMPositionBuilderAtEnd(builder, bb);
  emit_x(builder, a);
  LLVMBuildBr(builder, next);
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) verify function
/*----------------------------------------------------------------------------*/
void emit_condition_block(LLVMBuilderRef builder,
                          LLVMBasicBlockRef bb,
                          struct ast *a,
                          LLVMBasicBlockRef code_block,
                          LLVMBasicBlockRef continue_block)
{
  //fprintf(stderr, "in process - *** emit_condition_block\n");
  LLVMPositionBuilderAtEnd(builder, bb);
  LLVMValueRef result = emit_evaluate_expression(builder, a);
  LLVMBuildCondBr(builder, result, code_block, continue_block);
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
LLVMBasicBlockRef emit_for_loop(LLVMBuilderRef builder, struct ast *a)
{
  if (a == NULL) {
    fprintf(stderr, "Error - emit_for_loop - NULL\n");
    exit(1);
  }

  if (a->nodetype != N_for_loop) {
    fprintf(stderr, "Error - emit_for_loop - expected for loop, found %d\n", a->nodetype);
    exit(1);
  }

  struct forloop *fl = (struct forloop *) a;

  emit_assignment(builder, fl->initialize);

  LLVMValueRef fn = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
  LLVMBasicBlockRef last = LLVMGetLastBasicBlock(fn);

  LLVMBasicBlockRef code_block = LLVMAppendBasicBlock(fn, "code_block");
  LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(fn, "cond_block");
  LLVMBasicBlockRef post_block = LLVMAppendBasicBlock(fn, "post_block");
  LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(fn, "continue");

  LLVMBuildBr(builder, cond_block);

  fprintf(stderr, "-------------------- for loop code block ---------------\n\n");
  emit_code_block(builder, code_block, fl->codeblock, post_block);
  fprintf(stderr, "-------------------- for loop condition block ---------------\n\n");
  emit_condition_block(builder, cond_block, fl->condition, code_block, continue_block);
  fprintf(stderr, "-------------------- for loop post block ---------------\n\n");
  emit_code_block(builder, post_block, fl->post, cond_block);

  LLVMMoveBasicBlockAfter(code_block, cond_block);
  LLVMMoveBasicBlockAfter(post_block, code_block);
  LLVMMoveBasicBlockAfter(continue_block, post_block);
  LLVMMoveBasicBlockAfter(last, continue_block);

  LLVMPositionBuilderAtEnd(builder, continue_block);
  LLVMBuildBr(builder, last);

  fprintf(stderr, "-------------------- for loop returning contiue block ---------------\n\n");

  return continue_block;
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
void emit_x(LLVMBuilderRef builder, struct ast *a)
{
  if (a == NULL) {
    return;
  }

  switch (a->nodetype) {
    case N_func_def:
      emit_x(builder, ((struct funcdef *) a)->body);
    case N_var_declarations:
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    case N_var_declaration:
      emit_var_declaration(builder, a);
      break;
    case N_assignment:
      //fprintf(stderr, "emit_x - in process - N_assignment\n");
      emit_assignment(builder, a);
      break;
    case N_condition:
      fprintf(stderr, "emit_x - work on next - N_condition\n");
      break;
    case N_integer:
      fprintf(stderr, "emit_x - work on next - N_integer\n");
      break;
    case N_float:
      fprintf(stderr, "emit_x - work on next - N_float\n");
      break;
    case N_symbol_ref:
      fprintf(stderr, "emit_x - work on next - N_symbol_ref\n");
      break;
    case N_array_ref:
      fprintf(stderr, "emit_x - work on next - N_array_ref\n");
      break;
    case N_statement_list:
      fprintf(stderr, "emit_x - work on next - N_statement_list\n");
      break;
    case N_for_loop:
      emit_for_loop(builder, a);
      break;
    case N_while_loop:
      fprintf(stderr, "emit_x - work on next - N_while_loop\n");
      break;
    case N_until_loop:
      fprintf(stderr, "emit_x - work on next - N_until_loop\n");
      break;
    case N_add:
      fprintf(stderr, "emit_x - work on next - N_add\n");
      break;
    case N_subtract:
      fprintf(stderr, "emit_x - work on next - N_subtract\n");
      break;
    case N_multiply:
      fprintf(stderr, "emit_x - work on next - N_multiply\n");
      break;
    case N_divide:
      fprintf(stderr, "emit_x - work on next - N_divide\n");
      break;
    case N_scope:
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    case N_negate:
      fprintf(stderr, "emit_x - work on next - N_negate\n");
      break;
    default:
      fprintf(stderr, "emit_x - unexpected nodetype %d\n", a->nodetype);
  }
}
