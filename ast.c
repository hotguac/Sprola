/* Includes source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "ast.h"

extern void yyerror(char const*);
extern char *current_filename;   // read source from here
extern char *output_filename;    // write .ll output here
extern struct symbol symtab[NHASH];

void yyerror2(char *s, ...);

struct ast *newprogram(struct ast *options, struct ast *decls, struct ast *funcs)
{
  struct prog *a = (struct prog *) malloc(sizeof(struct prog));

  if (!a) {
    yyerror2("out of space");
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
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->l = l;
  a->r = r;

  return a;
}

struct ast *newoption(enum option_flags flag, struct ast* sym)
{
  struct setopt *a = (struct setopt *) malloc(sizeof(struct setopt));

  if (!a) {
    yyerror2("out of space");
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
    yyerror2("out of space");
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
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = N_assignment;
  a->target = target;
  a->value = value;

  return (struct ast *)a;
}

struct ast *newsymref(struct symbol *s)
{
  struct symref *a = (struct symref *) malloc(sizeof(struct symref));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = N_symbol_ref;
  a->sym = s;

  return (struct ast *)a;
}

struct ast *newarrayref(struct symbol *sym, struct ast *value)
{
  struct arrayref *a = (struct arrayref *) malloc(sizeof(struct symasgn));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = N_array_ref;
  a->sym = sym;
  a->index = value;

  return (struct ast *)a;
}

struct ast *newfunction(int return_type, struct ast* name, struct ast* code)
{
  struct funcdef *a = (struct funcdef *) malloc(sizeof(struct funcdef));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = N_func_def;
  a->name = (struct symref *) name;
  a->body = code;
  a->arglist = NULL;

  return (struct ast *)a;
}

struct ast *newlessthan(struct ast *left, struct ast *right)
{
  struct condition *a = (struct condition *) malloc(sizeof(struct condition));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = N_condition;
  a->operator = L_less_than;
  a->left = left;
  a->right = right;

  return (struct ast *)a;
}

struct ast *newforloop(struct ast *init, struct ast *cond, struct ast *post, struct ast *block)
{
  struct forloop *a = (struct forloop *) malloc(sizeof(struct forloop));

  if (!a) {
    yyerror2("out of space");
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
void emit_globals(FILE * codeout)
{
  printf("Emit Globals:\n");

  struct symbol *sp;

  for (sp = symtab; sp < symtab+NHASH; sp++) {
    if (sp->name) {

    /* now print the word and its references */
    fprintf(codeout,"@%s = global i32 0, align 4\n", sp->name);
    }
  }

  fprintf(codeout,"\n");

}
/*----------------------------------------------------------------------------*/
void emit_code(struct ast *a)
{
FILE * codeout;

printf("output filename = '%s'\n", output_filename);

if (!(codeout = fopen(output_filename, "w"))) {
  perror(output_filename);
}

// Header code here
fprintf(codeout,"; ModuleID = '%s'\n", current_filename);
fprintf(codeout,"target datalayout = \"e-m:e-i64:64-f80:128-n8:16:32:64-S128\"\n");
fprintf(codeout,"target triple = \"x86_64-pc-linux-gnu\"\n");
fprintf(codeout,"\n");

// Define global variables here
emit_globals(codeout);

// start definition of main
fprintf(codeout,"define i32 @main(i32 %%argc, i8** %%argv) #0 {\n");

// Body code here


// Trailer code here
fprintf(codeout,"  ret i32 @x\n");
fprintf(codeout,"}\n");
fprintf(codeout,"\n");
fprintf(codeout,"attributes #0 = { nounwind uwtable ");
fprintf(codeout,"\"disable-tail-calls\"=\"false\" ");
fprintf(codeout,"\"less-precise-fpmad\"=\"false\" ");
fprintf(codeout,"\"no-frame-pointer-elim\"=\"true\" ");
fprintf(codeout,"\"no-frame-pointer-elim-non-leaf\" ");
fprintf(codeout,"\"no-infs-fp-math\"=\"false\" \"no-nans-fp-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+fxsr,+mmx,+sse,+sse2\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }\n");
fprintf(codeout,"\n");


fprintf(codeout,"\n");
fprintf(codeout,"!llvm.module.flags = !{!0}\n");
fprintf(codeout,"!llvm.ident = !{!1}\n");
fprintf(codeout,"\n");
fprintf(codeout,"!0 = !{i32 1, !\"PIC Level\", i32 2}\n");
fprintf(codeout,"!1 = !{!\"sprola version 0.0.1-1 (tags/RELEASE_001/final)\"}\n");
fprintf(codeout,"\n");

fclose(codeout);
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
      printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */
}

/*----------------------------------------------------------------------------*/
void yyerror2(char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
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
      }
      return;

    case N_options:
      printf("options\n");
      dumpast(a->l, level);
      dumpast(a->r, level);
      return;

    case N_condition:
      printf("cond\n");
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
      printf("add left\n");
      dumpast(a->l, level);
      printf("add right\n");
      dumpast(a->r, level);
      return;

    case N_subtract:
      printf("sub\n");
      return;

    case N_multiply:
      printf("mult\n");
      return;

    case N_divide:
      printf("divide\n");
      return;

    case N_scope:
      printf("scope\n");
      return;

    case N_negate:
      printf("neg\n");
      return;


    default: printf("bad %c\n", a->nodetype);
      return;
  }

}
