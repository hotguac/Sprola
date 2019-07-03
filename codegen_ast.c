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
LLVMValueRef emit_array_ref(LLVMBuilderRef builder, struct ast *a)
{
  struct arrayref *aref;
  struct symbol *sym;

  fprintf(stderr, "In emit_array_ref\n");

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_array_ref\n");
    exit(1);
  }

  aref = (struct arrayref *) a;
  sym = aref->sym;

  if (a->nodetype != N_array_ref) {
    fprintf(stderr, "Error - in emit_array_ref - expected nodetype N_array_ref, found %d\n", a->nodetype);
  }

  LLVMValueRef i = emit_evaluate_expression(builder, aref->index);
  LLVMSetAlignment(i, 4);
  LLVMValueRef ind = LLVMBuildZExt(builder, i, LLVMInt64Type(), "");
  LLVMValueRef x = LLVMBuildLoad(builder, sym->value, "");
  LLVMSetAlignment(x, 8);

  LLVMValueRef indexes[] = { ind };
  LLVMValueRef y = LLVMBuildInBoundsGEP(builder, x, indexes, 1, "");
  return y;
  //return LLVMBuildLoad(builder, y, "");
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_evaluate_expression(LLVMBuilderRef builder, struct ast *a)
{
  struct intval *i;
  struct symref *ref;
  struct symbol *sym;

  LLVMValueRef left;
  LLVMValueRef right;

  LLVMValueRef temp;

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_evaluate_expression\n");
    exit(1);
  }

  switch (a->nodetype) {
    case N_condition:
      return emit_evaluate_condition(builder,a);
      break;
    case N_integer:
      i = (struct intval *)a;
      return LLVMConstInt(LLVMInt32Type(), i->number, 0);
      break;
    case N_float:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_float\n");
      break;
    case N_symbol_ref:
      ref = (struct symref *) a;
      sym = ref->sym;
      return LLVMBuildLoad(builder, sym->value, "");
      break;
    case N_array_ref:
      temp = emit_array_ref(builder, a);
      return LLVMBuildLoad(builder, temp, "");
      break;
    case N_add:
      left = emit_evaluate_expression(builder, a->l);
      right = emit_evaluate_expression(builder, a->r);
      return LLVMBuildAdd(builder, left, right, "sum");
      break;
    case N_subtract:
      fprintf(stderr, "****in process - emit_evaluate_expression - work on next - N_subtract\n");
      break;
    case N_multiply:
      left = emit_evaluate_expression(builder, a->l);
      right = emit_evaluate_expression(builder, a->r);
      temp = LLVMBuildFMul(builder, left, right, "product");
      return temp;
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
  LLVMValueRef temp;

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
    case N_array_ref:
      temp = emit_array_ref(builder, a);
      return temp;
      break;
    default:
      fprintf(stderr, "in get_target found unknown nodetype %d\n", a->nodetype);
      return NULL;
  }
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) verify function
/*----------------------------------------------------------------------------*/
void emit_assignment( LLVMBuilderRef builder, struct ast *a)
{
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

  LLVMBuildStore(builder, value, target);
}

/*----------------------------------------------------------------------------*/
//TODO(jkokosa) finish function
/*----------------------------------------------------------------------------*/
void emit_code_block( LLVMBuilderRef builder, LLVMBasicBlockRef bb,
                      struct ast *a, LLVMBasicBlockRef next)
{
  //fprintf(stderr, "in process - emit_code_block\n");
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

  emit_code_block(builder, code_block, fl->codeblock, post_block);
  emit_condition_block(builder, cond_block, fl->condition, code_block, continue_block);
  emit_code_block(builder, post_block, fl->post, cond_block);

  LLVMMoveBasicBlockAfter(code_block, cond_block);
  LLVMMoveBasicBlockAfter(post_block, code_block);
  LLVMMoveBasicBlockAfter(continue_block, post_block);
  LLVMMoveBasicBlockAfter(last, continue_block);

  LLVMPositionBuilderAtEnd(builder, continue_block);
  LLVMBuildBr(builder, last);

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
      fprintf(stderr, "emit_x - inprocess - N_statement_list\n");
      emit_x(builder, a->l);
      emit_x(builder, a->r);
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
