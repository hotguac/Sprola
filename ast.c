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

void yyerror2(char *s, ...);

/*----------------------------------------------------------------------------*/
struct ast * newast(int nodetype, struct ast *l, struct ast *r)
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

/*----------------------------------------------------------------------------*/
struct ast * newnum(double d)
{
  struct numval *a = (struct numval *) malloc(sizeof(struct numval));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = 'K';
  a->number = d;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newcmp(int cmptype, struct ast *l, struct ast *r)
{
  struct ast *a = (struct ast *) malloc(sizeof(struct ast));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;

  return a;
}

/*----------------------------------------------------------------------------*/
// struct ast * newfunc(int functype, struct ast *l)
struct ast * newfunc(enum bifs functype, struct ast *l)
{
  struct fncall *a = (struct fncall *) malloc(sizeof(struct fncall));

  if (!a) {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newcall(struct symbol *s, struct ast *l)
{
  struct ufncall *a = (struct ufncall *) malloc(sizeof(struct ufncall));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = 'C';
  a->l = l;
  a->s = s;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newref(struct symbol *s)
{
  struct symref *a = (struct symref *) malloc(sizeof(struct symref));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = 'N';
  a->s = s;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newasgn(struct symbol *s, struct ast *v)
{
  struct symasgn *a = (struct symasgn *) malloc(sizeof(struct symasgn));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = '=';
  a->s = s;
  a->v = v;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct ast * newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
  struct flow *a = (struct flow *) malloc(sizeof(struct flow));

  if (!a) {
    yyerror2("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;

  return (struct ast *)a;
}

/*----------------------------------------------------------------------------*/
struct symlist * newsymlist(struct symbol *sym, struct symlist *next)
{
  struct symlist *sl = (struct symlist *) malloc(sizeof(struct symlist));

  if (!sl) {
    yyerror2("out of space");
    exit(0);
  }

  sl->sym = sym;
  sl->next = next;

  return sl;
}

/*----------------------------------------------------------------------------*/
void symlistfree(struct symlist *sl)
{
  struct symlist *nsl;

  while (sl) {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}

/*----------------------------------------------------------------------------*/
/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
  if (name->syms) {
    symlistfree(name->syms);
  }

  if (name->func) {
    treefree(name->func);
  }

  name->syms = syms;
  name->func = func;
}

static double callbuiltin(struct fncall *);
static double calluser(struct ufncall *);

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
fprintf(codeout,"@.str = private unnamed_addr constant [7 x i8] c\"Holle!\\00\", align 1\n");

// Body code here
fprintf(codeout,"define i32 @main(i32 %%argc, i8** %%argv) #0 {\n");
fprintf(codeout,"  ret i32 0\n");
fprintf(codeout,"}\n");
fprintf(codeout,"\n");
fprintf(codeout,"attributes #0 = { nounwind uwtable ");
fprintf(codeout,"\"disable-tail-calls\"=\"false\" ");
fprintf(codeout,"\"less-precise-fpmad\"=\"false\" ");
fprintf(codeout,"\"no-frame-pointer-elim\"=\"true\" ");
fprintf(codeout,"\"no-frame-pointer-elim-non-leaf\" ");
fprintf(codeout,"\"no-infs-fp-math\"=\"false\" \"no-nans-fp-math\"=\"false\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+fxsr,+mmx,+sse,+sse2\" \"unsafe-fp-math\"=\"false\" \"use-soft-float\"=\"false\" }\n");
fprintf(codeout,"\n");


// Trailer code here
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
double eval(struct ast *a)
{
  double v;

  if (!a) {
    yyerror("internal error, null eval");
    return 0.0;
  }

  switch(a->nodetype) {
    /* constant */
    case 'K':
      v = ((struct numval *)a)->number;
      break;

    /* name reference */
    case 'N':
      // v = ((struct symref *)a)->s->value;
      break;

    /* assignment */
    case '=':
      // v = ((struct symasgn *)a)->s->value =
      //  eval(((struct symasgn *)a)->v);
      break;

    /* expressions */
    case '+':
      v = eval(a->l) + eval(a->r);
      break;

    case '-':
      v = eval(a->l) - eval(a->r);
      break;

    case '*':
      v = eval(a->l) * eval(a->r);
      break;

    case '/':
      v = eval(a->l) / eval(a->r);
      break;

    case '|':
      v = fabs(eval(a->l));
      break;

    case 'M':
      v = -eval(a->l);
      break;

    /* comparisons */
    case '1':
      v = (eval(a->l) > eval(a->r))? 1 : 0;
      break;

    case '2':
      v = (eval(a->l) < eval(a->r))? 1 : 0;
      break;

    case '3':
      v = (eval(a->l) != eval(a->r))? 1 : 0;
      break;

    case '4':
      v = (eval(a->l) == eval(a->r))? 1 : 0;
      break;

    case '5':
      v = (eval(a->l) >= eval(a->r))? 1 : 0;
      break;

    case '6':
      v = (eval(a->l) <= eval(a->r))? 1 : 0;
      break;

  /* control flow */
  /* null if/else/do expressions allowed in the grammar, so check for them */
    case 'I':
      if ( eval( ((struct flow *)a)->cond) != 0) {
        if ( ((struct flow *)a)->tl) {
	         v = eval( ((struct flow *)a)->tl);
        } else
	         v = 0.0;		/* a default value */
      } else {
        if ( ((struct flow *)a)->el) {
          v = eval(((struct flow *)a)->el);
        } else
	         v = 0.0;		/* a default value */
      }
      break;

    case 'W':
      v = 0.0;		/* a default value */

      if ( ((struct flow *)a)->tl) {
        while ( eval(((struct flow *)a)->cond) != 0)
	         v = eval(((struct flow *)a)->tl);
      }
      break;			/* last value is value */

    case 'L':
      eval(a->l);
      v = eval(a->r);
      break;

    case 'F':
      v = callbuiltin((struct fncall *)a);
      break;

    case 'C':
      v = calluser((struct ufncall *)a);
      break;

    default: printf("internal error: bad node %c\n", a->nodetype);
  }

  return v;
}

/*----------------------------------------------------------------------------*/
static double callbuiltin(struct fncall *f)
{
  enum bifs functype = f->functype;
  double v = eval(f->l);

  switch (functype) {
    case B_sqrt:
      return sqrt(v);
    case B_exp:
      return exp(v);
    case B_log:
      return log(v);
    case B_print:
      printf("= %4.4g\n", v);
      return v;
    default:
      yyerror("Unknown built-in function %d"); // , functype);
      return 0.0;
  }
}

/*----------------------------------------------------------------------------*/
static double calluser(struct ufncall *f)
{
  struct symbol *fn = f->s;	/* function name */
  struct symlist *sl;		/* dummy arguments */
  struct ast *args = f->l;	/* actual arguments */
  double *oldval, *newval;	/* saved arg values */
  double v;
  int nargs;
  int i;

  if (!fn->func) {
    yyerror("call to undefined function"); // , fn->name);
    return 0;
  }

  /* count the arguments */
  sl = fn->syms;
  for(nargs = 0; sl; sl = sl->next) {
    nargs++;
  }

  /* prepare to save them */
  oldval = (double *)malloc(nargs * sizeof(double));
  newval = (double *)malloc(nargs * sizeof(double));

  if (!oldval || !newval) {
    yyerror("Out of space in %s"); // , fn->name);
    return 0.0;
  }

  /* evaluate the arguments */
  for (i = 0; i < nargs; i++) {
    if (!args) {
      yyerror("too few args in call to %s"); // , fn->name);
      free(oldval); free(newval);
      return 0;
    }

    if (args->nodetype == 'L') {	/* if this is a list node */
      newval[i] = eval(args->l);
      args = args->r;
    } else {			/* if it's the end of the list */
      newval[i] = eval(args);
      args = NULL;
    }
  }

  /* save old values of dummies, assign new ones */
  sl = fn->syms;
  for (i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    // oldval[i] = s->value;
    // s->value = newval[i];
    sl = sl->next;
  }

  free(newval);

  /* evaluate the function */
  v = eval(fn->func);

  /* put the dummies back */
  sl = fn->syms;
  for (i = 0; i < nargs; i++) {
    struct symbol *s = sl->sym;

    // s->value = oldval[i];
    sl = sl->next;
  }

  free(oldval);
  return v;
}

/*----------------------------------------------------------------------------*/
void treefree(struct ast *a)
{
  switch(a->nodetype) {

    /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
  case 'L':
    treefree(a->r);

    /* one subtree */
  case '|':
  case 'M': case 'C': case 'F':
    treefree(a->l);

    /* no subtree */
  case 'K': case 'N':
    break;

  case '=':
    free( ((struct symasgn *)a)->v);
    break;

  case 'I': case 'W':
    free( ((struct flow *)a)->cond);
    if( ((struct flow *)a)->tl) free( ((struct flow *)a)->tl);
    if( ((struct flow *)a)->el) free( ((struct flow *)a)->el);
    break;

  default: printf("internal error: free bad node %c\n", a->nodetype);
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
  printf("%*s", 2*level, "");	/* indent to this level */
  level++;

  if (!a) {
    printf("NULL\n");
    return;
  }

  switch(a->nodetype) {
    /* constant */
  case 'K': printf("number %4.4g\n", ((struct numval *)a)->number); break;

    /* name reference */
  case 'N': printf("ref %s\n", ((struct symref *)a)->s->name); break;

    /* assignment */
  case '=': printf("= %s\n", ((struct symref *)a)->s->name);
    dumpast( ((struct symasgn *)a)->v, level); return;

    /* expressions */
  case '+': case '-': case '*': case '/': case 'L':
  case '1': case '2': case '3':
  case '4': case '5': case '6':
    printf("binop %c\n", a->nodetype);
    dumpast(a->l, level);
    dumpast(a->r, level);
    return;

  case '|': case 'M':
    printf("unop %c\n", a->nodetype);
    dumpast(a->l, level);
    return;

  case 'I': case 'W':
    printf("flow %c\n", a->nodetype);
    dumpast( ((struct flow *)a)->cond, level);
    if( ((struct flow *)a)->tl)
      dumpast( ((struct flow *)a)->tl, level);
    if( ((struct flow *)a)->el)
      dumpast( ((struct flow *)a)->el, level);
    return;

  case 'F':
    printf("builtin %d\n", ((struct fncall *)a)->functype);
    dumpast(a->l, level);
    return;

  case 'C': printf("call %s\n", ((struct ufncall *)a)->s->name);
    dumpast(a->l, level);
    return;

  default: printf("bad %c\n", a->nodetype);
    return;
  }
}
