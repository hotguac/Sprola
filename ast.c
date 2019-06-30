/* Includes source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
struct ast *newoption(enum option_flags flag, struct ast* sym)
{
  struct setopt *a = (struct setopt *) malloc(sizeof(struct setopt));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = N_option;
  a->option_flag = flag;
  a->target = sym;

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
int debug = 0;
void dumpast(struct ast *a, int level)
{
  struct symasgn *node_assign;
  struct intval *node_int;
  struct prog *node_prog;
  struct funcdef *node_funcdef;
  struct floatval *node_float;
  struct forloop *node_forloop;
  struct arrayref *node_arrayref;
  struct setopt *node_option;
  struct condition *node_cond;
  struct var_decl *node_var_decl;

  printf("%2d%*s", level, 2*level, "");	/* indent to this level */
  level++;

  if (!a) {
    printf(".\n");
    return;
  }

  switch(a->nodetype) {
    case N_program:
      node_prog = (struct prog *) a;
      printf("program\n\n");
      dumpast(node_prog->opts, level);
      printf("\n");
      dumpast(node_prog->decls, level);
      printf("\n");
      dumpast(node_prog->funcs, level);
      return;

    case N_functions:
      printf("functions\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_func_def:
      node_funcdef = (struct funcdef *) a;
      printf("funcdef %d %s\n", node_funcdef->return_type,
                                node_funcdef->name->sym->name);
      dumpast(node_funcdef->body, level);
      return;

    case N_options:
      printf("options\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_option:
      node_option = (struct setopt *) a;
      switch (node_option->option_flag) {
        case OPT_lv2:
          printf("option lv2\n");
          dumpast(node_option->target,level);
          return;
        case OPT_audio_input:
          printf("option audio input\n");
          dumpast(node_option->target,level);
          return;
        case OPT_audio_output:
          printf("option audio output\n");
          dumpast(node_option->target,level);
          return;
        case OPT_control_in:
          printf("option control input\n");
          dumpast(node_option->target,level);
          return;
        case OPT_control_out:
          printf("option control output\n");
          dumpast(node_option->target,level);
          return;
        case OPT_uri:
          printf("option uri\n");
          dumpast(node_option->target,level);
          return;
      }
      return;

    case N_condition:
      printf("cond\n");
      node_cond = (struct condition *) a;
      dumpast(node_cond->left, level);
      printf("%2d%*s", level, 2*level, "");	/* indent to this level */
      switch (node_cond->operator) {
        case L_less_than:
          printf(" < \n");
          break;
        case L_greater_than:
          printf(" > \n");
          break;
        case L_equals:
          printf(" = \n");
          break;
      }
      dumpast(node_cond->right, level);

      return;

    case N_float:
      node_float = (struct floatval *)a;
      printf("float %f\n", node_float->number);
      return;

    case N_array_ref:
      node_arrayref = (struct arrayref *)a;
      printf("arrayref %s\n", node_arrayref->sym->name);
      dumpast(node_arrayref->index, level);
      return;

    case N_for_loop:
      node_forloop = (struct forloop *)a;
      printf("for loop\n");
      dumpast(node_forloop->initialize, level);
      dumpast(node_forloop->condition, level);
      dumpast(node_forloop->post, level);
      dumpast(node_forloop->codeblock, level);
      return;

    case N_while_loop:
      printf("while loop\n");
      return;

    case N_until_loop:
      printf("until loop\n");
      return;

    case N_assignment:
      printf("assignment\n");
      node_assign = (struct symasgn *)a;
      dumpast(node_assign->target, level);
      dumpast( node_assign->value, level);
      return;

    case N_integer:
      node_int = (struct intval *)a;
      printf("int %d\n", node_int->number);
      return;

    case N_statement_list:
      printf("Statement list left side\n");
      dumpast(a->l, level);
      printf("%*s", 2*level, "");	/* indent to this level */
      printf("Statement list right side\n");
      dumpast(a->r, level);
      return;

    case N_symbol_ref:
      printf("Symbol Reference: %s\n", ((struct symref *)a)->sym->name);
      return;

    case N_add:
      printf("addition\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_subtract:
      printf("sub\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_multiply:
      printf("mult\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_divide:
      printf("divide\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_scope:
      printf("scope\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_var_declarations:
      printf("var declares\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_var_declaration:
      node_var_decl = (struct var_decl *)a;
      printf("var decl\n");
      dumpast(node_var_decl->sym, level);
      return;

    case N_negate:
      printf("neg\n");
      return;


    default: printf("bad %d\n", a->nodetype);
      return;
  }

}
