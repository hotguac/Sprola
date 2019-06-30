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
    fprintf(stderr, "name = %s\n", s->name);
    fprintf(stderr, "family = %d\n", s->sym_family);
    fprintf(stderr, "type = %s\n", s->sym_typ);
  }

  if (s->sym_family != VARIABLE_NAME) {
    fprintf(stderr, "Error - emit_var_declaration - expecting variable declaration");
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

}

/*----------------------------------------------------------------------------*/
void emit_function_def(LLVMBuilderRef builder, struct ast *a)
{
  if (a == NULL) {
    return;
  }

  switch (a->nodetype) {
    case N_func_def:
      emit_function_def(builder, ((struct funcdef *) a)->body);
    case N_var_declarations:
      emit_function_def(builder, a->l);
      emit_function_def(builder, a->r);
      break;
    case N_var_declaration:
      emit_var_declaration(builder, a);
      break;
    case N_assignment:
      break;
    case N_condition:
      break;
    case N_integer:
      break;
    case N_float:
      break;
    case N_symbol_ref:
      break;
    case N_array_ref:
      break;
    case N_statement_list:
      break;
    case N_for_loop:
      break;
    case N_while_loop:
      break;
    case N_until_loop:
      break;
    case N_add:
      break;
    case N_subtract:
      break;
    case N_multiply:
      break;
    case N_divide:
      break;
    case N_scope:
      emit_function_def(builder, a->l);
      emit_function_def(builder, a->r);
      break;
    case N_negate:
      break;
    default:
      fprintf(stderr, "unexpected nodetype %d\n", a->nodetype);
  }
}
