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

/*----------------------------------------------------------------------------*/
struct ast * newast(enum node_types nodetype, struct ast *l, struct ast *r)
{
  printf("newast:\n");
  printf("\tnodetype = '%c'\n", nodetype);

  if (!l) {
    printf("\tleft nodetype = NULL\n'\n");
  } else {
    printf("\tleft nodetype = '%c'\n", l->nodetype);
  }

  if (!r) {
    printf("\tright nodetype = NULL\n");
  } else {
    printf("\tright nodetype = '%c'\n", r->nodetype);
  }

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

/*----------------------------------------------------------------------------*/
struct ast * newint(int d)
{
  printf("newint:\n");
  printf("\tvalue = ''%d'\n", d);

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
struct ast * newasgn(struct symbol *s, struct ast *v)
{
  printf("newasgn:\n");
  printf("\tsymbol = '%s'\n", s->name);
  printf("\tright hand nodetype = '%c'\n", v->nodetype);

  struct symasgn *a = (struct symasgn *) malloc(sizeof(struct symasgn));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = N_assignment;
  a->s = s;
  a->v = v;

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
      free( ((struct symasgn *)a)->v);
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
  struct symasgn *node_assign = (struct symasgn *)a;
  struct intval *node_int = (struct intval *)a;

  printf("%*s", 2*level, "");	/* indent to this level */
  level++;

  if (!a) {
    printf("NULL\n");
    return;
  }

  switch(a->nodetype) {
    /* assignment */
    case N_assignment:
      node_assign = (struct symasgn *)a;
      printf("%s = ", node_assign->s->name);
      dumpast( node_assign->v, level);
      return;

    case N_integer:
      node_int = (struct intval *)a;
      printf("%d\n", node_int->number);
      return;

    case N_statement_list:
      printf("Statement list left side\n");
      dumpast(a->l, level);
      printf("%*s", 2*level, "");	/* indent to this level */
      printf("Statement list right side\n");
      dumpast(a->r, level);
      return;

    case N_symbol_ref:
      printf("Symbol Reference: %s\n", ((struct symref *)a)->s->name);
      return;

    case N_add:
      printf("add left\n");
      dumpast(a->l, level);
      printf("add right\n");
      dumpast(a->r, level);
      return;

    case N_subtract:
    case N_multiply:
    case N_divide:
    case N_scope:
    case N_negate:
      return;

    default: printf("bad %c\n", a->nodetype);
      return;
  }

}
