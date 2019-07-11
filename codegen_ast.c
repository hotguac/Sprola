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
    fprintf(trace_file, "emit_var_declaration\n");
    fflush(trace_file);
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
    fprintf(trace_file,"emit_evaluate_condition\n");
    fflush(trace_file);
  }

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_evaluate_condition\n");
  }

  struct condition *c = (struct condition *) a;

  LLVMValueRef left = emit_evaluate_expression(builder, c->left);
  //fprintf(stderr, "eec left = %s\n", LLVMPrintValueToString(left));

  LLVMValueRef right = emit_evaluate_expression(builder, c->right);
  //fprintf(stderr, "eec right = %s\n", LLVMPrintValueToString(right));

  LLVMTypeKind tk_l = LLVMGetTypeKind(LLVMTypeOf(left));
  LLVMTypeKind tk_r = LLVMGetTypeKind(LLVMTypeOf(right));

  //TODO(jkokosa) assuming only int and float are valid types
  if (tk_l != tk_r) {
    if (tk_l == LLVMIntegerTypeKind) {
      left = LLVMBuildSIToFP(builder, left, LLVMFloatType(), "");
      tk_l = LLVMGetTypeKind(LLVMTypeOf(left));
    }
    if (tk_r == LLVMIntegerTypeKind) {
      right = LLVMBuildSIToFP(builder, right, LLVMFloatType(), "");
    }
  }

  switch (c->operator) {
    case L_greater_than:
      if (tk_l == LLVMIntegerTypeKind) {
        result = LLVMBuildICmp(builder, LLVMIntSGT, left, right, "");
      } else {
        result = LLVMBuildFCmp(builder, LLVMRealUGT, left, right, "");
      }
      break;
    case L_less_than:
      if (tk_l == LLVMIntegerTypeKind) {
        result = LLVMBuildICmp(builder, LLVMIntSLT, left, right, "");
      } else {
        result = LLVMBuildFCmp(builder, LLVMRealULT, left, right, "");
      }
      break;
    case L_equals:
      if (tk_l == LLVMIntegerTypeKind) {
        result = LLVMBuildICmp(builder, LLVMIntEQ, left, right, "");
      } else {
        result = LLVMBuildFCmp(builder, LLVMRealUEQ, left, right, "");
      }
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
    fprintf(trace_file, "In emit_array_ref\n");
    fflush(trace_file);
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
    fprintf(trace_file, "emit_basic_bin_operator\n");
    fflush(trace_file);
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
LLVMValueRef emit_basic_unary_operator(LLVMBuilderRef builder, struct ast *a)
{
  LLVMValueRef left;

  if (trace_flag) {
    fprintf(trace_file, "emit_basic_unary_operator\n");
    fflush(trace_file);
  }

  if (a == NULL) {
    fprintf(stderr, "Error - null ast in emit_basic_unary_operator\n");
    exit(1);
  }

  left = emit_evaluate_expression(builder, a->l);

  LLVMTypeKind tk_l = LLVMGetTypeKind(LLVMTypeOf(left));

  switch (a->nodetype) {
    case N_negate:
      if (tk_l == LLVMIntegerTypeKind) {
        return LLVMBuildNeg(builder, left, "neg");
      } else {
        return LLVMBuildFNeg(builder, left, "neg");
      }
      break;
    default:
      fprintf(stderr, "****in process - emit_basic_unary_operator - unexpected nodetype %d\n", a->nodetype);
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
    fprintf(trace_file, "emit_evaluate_expression\n");
    fflush(trace_file);
    dumpast(a, 0);
    fflush(trace_file);
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
      if (sym->value == NULL) {
        fprintf(stderr, "Error - undefined symbol %s\n", sym->name);
        dumpast(a, 0);
        exit(1);
      }
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
      if (trace_flag) {
        fprintf(trace_file, "emit_evaluate_expression - in process N_negate\n");
      }

      return emit_basic_unary_operator(builder, a);
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
    fprintf(trace_file, "get_target\n");
    fflush(trace_file);
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
  LLVMValueRef temp_value;

  trace("emit_assignment\n");

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

  LLVMTypeKind tk_t = LLVMGetTypeKind(LLVMTypeOf(target));
  LLVMTypeKind tk_v = LLVMGetTypeKind(LLVMTypeOf(value));

  if (trace_flag) {
    fprintf(trace_file, "tk_t = %d\n", tk_t);
    fprintf(trace_file, "tk_v = %d\n", tk_v);

    fprintf(trace_file, "target = '%s'\n", LLVMPrintValueToString(target));
    fprintf(trace_file, "value = '%s'\n", LLVMPrintValueToString(value));
  }

  if (tk_t == LLVMPointerTypeKind) {
    tk_t = LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(target)));
    fprintf(trace_file, "tk_t = %d\n", tk_t);
  }

  //TODO(jkokosa) assuming on int and float are only valid types
  if (tk_t != tk_v) {
    if (tk_t == LLVMIntegerTypeKind) {
      temp_value = LLVMBuildFPToSI(builder, value, LLVMInt32Type(), "");
      trace("FPToSI\n");
    } else {
      trace("before SIToFP\n");
      temp_value = LLVMBuildSIToFP(builder, value, LLVMFloatType(), "");
      trace("SIToFP\n");
    }
  } else {
    temp_value = value;
  }

  trace("before build store assignment\n");
  fprintf(trace_file, "temp_value = '%s'\n", LLVMPrintValueToString(temp_value));
  LLVMBuildStore(builder, temp_value, target);
  //dump_current_function(builder);
  trace("after build store assignment\n");
  //dump_current_function(builder);
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
void emit_code_block( LLVMBuilderRef builder, LLVMBasicBlockRef bb,
                      struct ast *a, LLVMBasicBlockRef next)
{
  trace("in process - emit_code_block\n");
  dumpast(a, 0);

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
  trace("emit_condition_block\n");

  LLVMPositionBuilderAtEnd(builder, bb);
  LLVMValueRef result = emit_evaluate_expression(builder, a);
  LLVMBuildCondBr(builder, result, code_block, continue_block);
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
LLVMBasicBlockRef emit_if_else(LLVMBuilderRef builder, struct ast *a)
{
  trace("emit_if_else\n");

  if (a == NULL) {
    fprintf(stderr, "Error - emit_for_loop - NULL\n");
    exit(1);
  }

  if (a->nodetype != N_if_then_else) {
    fprintf(stderr, "Error - emit_if_else - expected if, found %d\n", a->nodetype);
    exit(1);
  }

  //dump_current_function(builder);

  struct ifthenelse *stmt = (struct ifthenelse *) a;

  LLVMValueRef fn = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
  //LLVMBasicBlockRef last = LLVMGetLastBasicBlock(fn);

  //fprintf(stderr, "eie last = %s\n", LLVMPrintValueToString(LLVMBasicBlockAsValue(last)));

  LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(fn, "cond_block");
  LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(fn, "then_block");
  LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(fn, "else_block");
  LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(fn, "continue");

  LLVMBuildBr(builder, cond_block);

  emit_condition_block(builder, cond_block, stmt->condition, then_block, else_block);
  emit_code_block(builder, then_block, stmt->then_block, continue_block);
  emit_code_block(builder, else_block, stmt->else_block, continue_block);

  LLVMMoveBasicBlockAfter(then_block, cond_block);
  LLVMMoveBasicBlockAfter(else_block, then_block);
  LLVMMoveBasicBlockAfter(continue_block, else_block);
  //LLVMMoveBasicBlockAfter(last, continue_block);

  //dump_current_function(builder);

  LLVMPositionBuilderAtEnd(builder, continue_block);
  //LLVMBuildBr(builder, last);

  return continue_block;
}

/*----------------------------------------------------------------------------*/
//
/*----------------------------------------------------------------------------*/
LLVMBasicBlockRef emit_for_loop(LLVMBuilderRef builder, struct ast *a)
{
  trace("emit_for_loop\n");

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
  LLVMBasicBlockRef exit_block = LLVMGetLastBasicBlock(fn);

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

  LLVMBasicBlockRef last_block = LLVMGetLastBasicBlock(fn);

  LLVMMoveBasicBlockAfter(continue_block, last_block);
  LLVMMoveBasicBlockAfter(exit_block, continue_block);

  LLVMPositionBuilderAtEnd(builder, continue_block);
  LLVMBuildBr(builder, exit_block);

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
      trace("emit_x N_func_def\n");
      emit_x(builder, ((struct funcdef *) a)->body);
      break;
    case N_var_declarations:
      trace("emit_x N_var_declarations\n");
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    case N_var_declaration:
      trace("emit_x N_var_declaration\n");
      emit_var_declaration(builder, a);
      break;
    case N_assignment:
      emit_assignment(builder, a);
      break;
    case N_statement_list:
      trace("emit_x N_statement_list\n");
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    case N_if_then_else:
      trace("emit_x N_if_then_else\n");
      emit_if_else(builder, a);
      break;
    case N_for_loop:
      trace("emit_x N_for_loop\n");
      emit_for_loop(builder, a);
      break;
    case N_while_loop:
      trace("emit_x N_while_loop - ** empty **\n");
      dumpast(a, 4);
      break;
    case N_until_loop:
      trace("emit_x N_while_loop - ** empty **\n");
      dumpast(a, 4);
      break;
    case N_scope:
      trace("emit_x N_scope\n");
      emit_x(builder, a->l);
      emit_x(builder, a->r);
      break;
    default:
      trace("emit_x default**********\n");
      dumpast(a, 4);
      break;
  }
}
