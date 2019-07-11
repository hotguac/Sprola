/* Includes source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "sprola.h"

void yyerror(char const*);

/*----------------------------------------------------------------------------*/
struct ast *newprogram(struct ast *options, struct ast *decls, struct ast *funcs)
{
  struct prog *a = (struct prog *) malloc(sizeof(struct prog));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_program;
  a->opts = options;
  a->decls = decls;
  a->funcs = funcs;

  return (struct ast *) a;
}

/*----------------------------------------------------------------------------*/
struct ast * newast(enum node_types nodetype, struct ast *l, struct ast *r)
{
  struct ast *a = (struct ast *) malloc(sizeof(struct ast));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->l = l;
  a->r = r;

  return a;
}

/*----------------------------------------------------------------------------*/
struct ast *newoption(enum option_flags flag, struct ast* sym, struct ast* value1, struct ast* value2, struct ast* value3)
{
  struct setopt *a = (struct setopt *) malloc(sizeof(struct setopt));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_option;
  a->option_flag = flag;
  a->target = sym;
  a->value1 = value1;
  a->value2 = value2;
  a->value3 = value3;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newint(int d)
{
  struct intval *a = (struct intval *) malloc(sizeof(struct intval));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_integer;
  a->number = d;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newfloat(float f)
{
  struct floatval *a = (struct floatval *) malloc(sizeof(struct floatval));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_float;
  a->number = f;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newasgn(struct ast *target, struct ast *value)
{
  struct symasgn *a = (struct symasgn *) malloc(sizeof(struct symasgn));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_assignment;
  a->target = target;
  a->value = value;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newsymref(struct symbol *s)
{
  struct symref *a = (struct symref *) malloc(sizeof(struct symref));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_symbol_ref;
  a->sym = s;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newvardecl(char *type, struct ast *s)
{
  struct var_decl *a = (struct var_decl *) malloc(sizeof(struct var_decl));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_var_declaration;
  a->sym = s;

  struct symref *ref;
  ref = (struct symref *) s;

  if (ref->sym->sym_typ != NULL) {
    fprintf(stderr, "Error - duplicate definition %s\n", ref->sym->name);
  }

  ref->sym->sym_family = VARIABLE_NAME;
  ref->sym->sym_typ = strdup(type);

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newarrayref(struct symbol *sym, struct ast *value)
{
  struct arrayref *a = (struct arrayref *) malloc(sizeof(struct arrayref));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_array_ref;
  a->sym = sym;
  a->index = value;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newfunction(int return_type, struct ast* name, struct ast* code)
{
  struct funcdef *a = (struct funcdef *) malloc(sizeof(struct funcdef));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_func_def;
  a->name = (struct symref *) name;
  a->body = code;
  a->arglist = NULL;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newlessthan(struct ast *left, struct ast *right)
{
  struct condition *a = (struct condition *) malloc(sizeof(struct condition));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_condition;
  a->operator = L_less_than;
  a->left = left;
  a->right = right;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newgreaterthan(struct ast *left, struct ast *right)
{
  struct condition *a = (struct condition *) malloc(sizeof(struct condition));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_condition;
  a->operator = L_greater_than;
  a->left = left;
  a->right = right;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newequals(struct ast *left, struct ast *right)
{
  struct condition *a = (struct condition *) malloc(sizeof(struct condition));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_condition;
  a->operator = L_equals;
  a->left = left;
  a->right = right;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast *newforloop(struct ast *init, struct ast *cond, struct ast *post, struct ast *block)
{
  struct forloop *a = (struct forloop *) malloc(sizeof(struct forloop));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_for_loop;
  a->initialize = init;
  a->condition = cond;
  a->post = post;
  a->codeblock = block;

  return (struct ast *)a;
}

struct ast *newifelse(struct ast *cond, struct ast *then_block, struct ast *else_block)
{
  struct ifthenelse *a = (struct ifthenelse *) malloc(sizeof(struct ifthenelse));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_if_then_else;
  a->condition = cond;
  a->then_block = then_block;
  a->else_block = else_block;

  return (struct ast *)a;
}


/*----------------------------------------------------------------------------*/
void treefree(struct ast *a)
{
  switch(a->nodetype) {
    /* two subtrees */
    case N_statement_list:
      treefree(a->r);

    case N_assignment:
      free( ((struct symasgn *)a)->value);
      break;

    default:
      fprintf(stderr, "internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */
}

/*----------------------------------------------------------------------------*/
/* debugging: dump out an AST */
//int debug = 0;
void dumpast(struct ast *a, int level)
{
  struct symasgn *node_assign;
  struct intval *node_int;
  struct prog *node_prog;
  struct funcdef *node_funcdef;
  struct floatval *node_float;
  struct forloop *node_forloop;
  struct ifthenelse *node_ifelse;
  struct arrayref *node_arrayref;
  struct setopt *node_option;
  struct condition *node_cond;
  struct var_decl *node_var_decl;

  FILE *dmp;

  if (trace_flag) {
    dmp = trace_file;
  } else {
    dmp = stdout;
  }

  fprintf(dmp, "%2d%*s", level, 2*level, "");	/* indent to this level */
  level++;

  if (!a) {
    fprintf(dmp, ".\n");
    return;
  }

  switch(a->nodetype) {
    case N_program:
      node_prog = (struct prog *) a;
      fprintf(dmp, "program\n\n");
      dumpast(node_prog->opts, level);
      fprintf(dmp, "\n");
      dumpast(node_prog->decls, level);
      printf("\n");
      dumpast(node_prog->funcs, level);
      return;

    case N_functions:
      fprintf(dmp, "functions\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_func_def:
      node_funcdef = (struct funcdef *) a;
      fprintf(dmp, "funcdef %d %s\n", node_funcdef->return_type,
                                node_funcdef->name->sym->name);
      dumpast(node_funcdef->body, level);
      return;

    case N_options:
      fprintf(dmp, "options\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_option:
      node_option = (struct setopt *) a;
      switch (node_option->option_flag) {
        case OPT_lv2:
          fprintf(dmp, "option lv2\n");
          dumpast(node_option->target,level);
          return;
        case OPT_audio_input:
          fprintf(dmp, "option audio input\n");
          dumpast(node_option->target,level);
          dumpast(node_option->value1, level);
          return;
        case OPT_audio_output:
          fprintf(dmp, "option audio output\n");
          dumpast(node_option->target,level);
          return;
        case OPT_control_in:
          fprintf(dmp, "option control input\n");
          dumpast(node_option->target,level);
          break;
        case OPT_control_out:
          fprintf(dmp, "option control output\n");
          dumpast(node_option->target,level);
          break;
        case OPT_uri:
          fprintf(dmp, "option uri\n");
          dumpast(node_option->target,level);
          return;
      }
      dumpast(node_option->value1, level);
      dumpast(node_option->value2, level);
      dumpast(node_option->value3, level);
      return;

    case N_condition:
      fprintf(dmp, "cond\n");
      node_cond = (struct condition *) a;
      dumpast(node_cond->left, level);
      fprintf(dmp, "%2d%*s", level, 2*level, "");	/* indent to this level */
      switch (node_cond->operator) {
        case L_less_than:
          fprintf(dmp, " < \n");
          break;
        case L_greater_than:
          fprintf(dmp, " > \n");
          break;
        case L_equals:
          fprintf(dmp, " = \n");
          break;
      }
      dumpast(node_cond->right, level);

      return;

    case N_float:
      node_float = (struct floatval *)a;
      fprintf(dmp, "float %f\n", node_float->number);
      return;

    case N_array_ref:
      node_arrayref = (struct arrayref *)a;
      fprintf(dmp, "arrayref %s\n", node_arrayref->sym->name);
      dumpast(node_arrayref->index, level);
      return;

    case N_for_loop:
      node_forloop = (struct forloop *)a;
      fprintf(dmp, "for loop\n");
      dumpast(node_forloop->initialize, level);
      dumpast(node_forloop->condition, level);
      dumpast(node_forloop->post, level);
      dumpast(node_forloop->codeblock, level);
      return;

    case N_if_then_else:
      node_ifelse = (struct ifthenelse *)a;
      fprintf(dmp, "if statement\n");
      dumpast(node_ifelse->condition, level);
      dumpast(node_ifelse->then_block, level);
      dumpast(node_ifelse->else_block, level);
      return;

    case N_while_loop:
      fprintf(dmp, "while loop\n");
      return;

    case N_until_loop:
      fprintf(dmp, "until loop\n");
      return;

    case N_assignment:
      fprintf(dmp, "assignment\n");
      node_assign = (struct symasgn *)a;
      dumpast(node_assign->target, level);
      dumpast( node_assign->value, level);
      return;

    case N_integer:
      node_int = (struct intval *)a;
      fprintf(dmp, "int %d\n", node_int->number);
      return;

    case N_statement_list:
      fprintf(dmp, "Statement list left side\n");
      dumpast(a->l, level);
      fprintf(dmp, "%*s", 2*level, "");	/* indent to this level */
      fprintf(dmp, "Statement list right side\n");
      dumpast(a->r, level);
      return;

    case N_symbol_ref:
      fprintf(dmp, "Symbol Reference: %s\n", ((struct symref *)a)->sym->name);
      return;

    case N_add:
      fprintf(dmp, "addition\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_subtract:
      fprintf(dmp, "sub\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_multiply:
      fprintf(dmp, "mult\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_divide:
      fprintf(dmp, "divide\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_scope:
      fprintf(dmp, "scope\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_var_declarations:
      fprintf(dmp, "var declares\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_var_declaration:
      node_var_decl = (struct var_decl *)a;
      fprintf(dmp, "var decl\n");
      dumpast(node_var_decl->sym, level);
      return;

    case N_negate:
      fprintf(dmp, "neg\n");
      dumpast(a->l, level);
      return;


    default: fprintf(dmp, "bad %d\n", a->nodetype);
      return;
  }

}
