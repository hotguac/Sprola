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
//
/*----------------------------------------------------------------------------*/
void emit_var_declaration(LLVMBuilderRef builder, struct ast *a)
{
  if (trace_flag) {
    fprintf(stderr, "emit_var_declaration\n");
  }

  if (a == NULL) {
    return;
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
//
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_evaluate_condition(LLVMBuilderRef builder, struct ast *a)
{
  LLVMValueRef result = NULL;

  if (trace_flag) {
    fprintf(stderr,"emit_evaluate_condition\n");
  }

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
//
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_array_ref(LLVMBuilderRef builder, struct ast *a)
{
  struct arrayref *aref;
  struct symbol *sym;

  if (trace_flag) {
    fprintf(stderr, "In emit_array_ref\n");
  }

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
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_basic_bin_operator(LLVMBuilderRef builder, struct ast *a)
{
  LLVMValueRef left;
  LLVMValueRef right;

  if (trace_flag) {
    fprintf(stderr, "emit_basic_bin_operator\n");
  }

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_basic_bin_operator\n");
    exit(1);
  }

  left = emit_evaluate_expression(builder, a->l);
  right = emit_evaluate_expression(builder, a->r);

  LLVMTypeKind tk_l = LLVMGetTypeKind(LLVMTypeOf(left));
  LLVMTypeKind tk_r = LLVMGetTypeKind(LLVMTypeOf(right));

  //TODO(jkokosa) assuming on int and float are valid types
  if (tk_l != tk_r) {
    if (tk_l == LLVMIntegerTypeKind) {
      left = LLVMBuildSIToFP(builder, left, LLVMFloatType(), "");
      tk_l = LLVMGetTypeKind(LLVMTypeOf(left));
    }
    if (tk_r == LLVMIntegerTypeKind) {
      right = LLVMBuildSIToFP(builder, right, LLVMFloatType(), "");
    }
  }

  switch (a->nodetype) {
    case N_add:
      if (tk_l == LLVMIntegerTypeKind) {
        return LLVMBuildAdd(builder, left, right, "sum");
      } else {
        return LLVMBuildFAdd(builder, left, right, "sum");
      }
      break;
    case N_subtract:
      if (tk_l == LLVMIntegerTypeKind) {
        return LLVMBuildSub(builder, left, right, "diff");
      } else {
        return LLVMBuildFSub(builder, left, right, "diff");
      }
      break;
    case N_multiply:
      if (tk_l == LLVMIntegerTypeKind) {
        return LLVMBuildMul(builder, left, right, "product");
      } else {
        return LLVMBuildFMul(builder, left, right, "product");
      }
      break;
    case N_divide:
      if (tk_l == LLVMIntegerTypeKind) {
        //return LLVMBuildDiv(builder, left, right, "quotient"); // ???? how to div int's
        break;
      } else {
        return LLVMBuildFDiv(builder, left, right, "quotient");
      }
      break;
    default:
      fprintf(stderr, "****in process - emit_basic_bin_operator - unexpected nodetype %d\n", a->nodetype);
  }

  // This is a dummy statement until function is ready
  return LLVMConstInt(LLVMInt32Type(), 0, 0);
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
LLVMValueRef emit_evaluate_expression(LLVMBuilderRef builder, struct ast *a)
{
  struct intval *i;
  struct floatval *f;
  struct symref *ref;
  struct symbol *sym;

  LLVMValueRef temp;

  if (trace_flag) {
    fprintf(stderr, "emit_evaluate_expression\n");
  }

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_evaluate_expression\n");
    exit(1);
  }

  switch (a->nodetype) {
    case N_condition:
      return emit_evaluate_condition(builder,a);
      break;
    case N_integer:
      i = (struct intval *) a;
      return LLVMConstInt(LLVMInt32Type(), i->number, 0);
      break;
    case N_float:
      f = (struct floatval *) a;
      return LLVMConstReal(LLVMFloatType(), f->number);
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
      return emit_basic_bin_operator(builder, a);
      break;
    case N_subtract:
      return emit_basic_bin_operator(builder, a);
      break;
    case N_multiply:
      return emit_basic_bin_operator(builder, a);
      break;
    case N_divide:
      return emit_basic_bin_operator(builder, a);
      break;
    case N_negate:
      fprintf(stderr, "emit_evaluate_expression - need N_negate\n");
      break;
    default:
      fprintf(stderr, "****in process - emit_evaluate_expression - unexpected nodetype %d\n", a->nodetype);
  }

  // This is a dummy statement until function is ready
  return LLVMConstInt(LLVMInt1Type(), 0, 0);
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
LLVMValueRef get_target(LLVMBuilderRef builder, struct ast *a)
{
  struct symref *ref;
  struct symbol *sym;
  LLVMValueRef temp;

  if (trace_flag) {
    fprintf(stderr, "get_target\n");
  }

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
//
/*----------------------------------------------------------------------------*/
void emit_assignment( LLVMBuilderRef builder, struct ast *a)
{
  if (trace_flag) {
    fprintf(stderr, "emit_assignment\n");
  }

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
//
/*----------------------------------------------------------------------------*/
void emit_code_block( LLVMBuilderRef builder, LLVMBasicBlockRef bb,
                      struct ast *a, LLVMBasicBlockRef next)
{
  if (trace_flag) {
    fprintf(stderr, "in process - emit_code_block\n");
  }

  LLVMPositionBuilderAtEnd(builder, bb);
  emit_x(builder, a);
  LLVMBuildBr(builder, next);
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
void emit_condition_block(LLVMBuilderRef builder,
                          LLVMBasicBlockRef bb,
                          struct ast *a,
                          LLVMBasicBlockRef code_block,
                          LLVMBasicBlockRef continue_block)
{
  if (trace_flag) {
    fprintf(stderr, "emit_condition_block\n");
  }

  LLVMPositionBuilderAtEnd(builder, bb);
  LLVMValueRef result = emit_evaluate_expression(builder, a);
  LLVMBuildCondBr(builder, result, code_block, continue_block);
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
LLVMBasicBlockRef emit_for_loop(LLVMBuilderRef builder, struct ast *a)
{
  if (trace_flag) {
    fprintf(stderr, "emit_for_loop\n");
  }

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
//
/*----------------------------------------------------------------------------*/
void emit_x(LLVMBuilderRef builder, struct ast *a)
{
  if (a == NULL) {
    return;
  }

  switch (a->nodetype) {
    case N_func_def:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_func_def\n");
      }
      emit_x(builder, ((struct funcdef *) a)->body);
      break;
    case N_var_declarations:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_var_declarations\n");
      }
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    case N_var_declaration:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_var_declaration\n");
      }
      emit_var_declaration(builder, a);
      break;
    case N_assignment:
      emit_assignment(builder, a);
      break;
    case N_statement_list:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_statement_list\n");
      }
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    case N_for_loop:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_for_loop\n");
      }
      emit_for_loop(builder, a);
      break;
    case N_while_loop:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_while_loop - ** empty **\n");
        dumpast(a, 4);
      }
      break;
    case N_until_loop:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_while_loop - ** empty **\n");
        dumpast(a, 4);
      }
      break;
    case N_scope:
      if (trace_flag) {
        fprintf(stderr, "emit_x N_scope\n");
      }
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    default:
      if (trace_flag) {
        fprintf(stderr, "emit_x default**********\n");
        dumpast(a, 4);
      }
      fprintf(stderr, "emit_x - unexpected nodetype %d\n", a->nodetype);
      break;
  }
}
