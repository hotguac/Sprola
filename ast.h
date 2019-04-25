/* Includes Source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/

#include "symbols.h"

extern struct symbol *lookup(char*);

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

/* node types
 *  + - * / |
 *  0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 *  M unary minus
 *  L statement list
 *  I IF statement
 *  W WHILE statement
 *  N symbol ref
 *  = assignment
 *  S list of symbols
 *  F built in function call
 *  C user function call
 */
enum node_types {
  N_assignment = '=',
  N_integer = 'i',
  N_symbol_ref = 'r',
  N_statement_list = 'L'
};

enum bifs {			/* built-in functions */
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print
};

/* nodes in the Abstract Syntax Tree */
/* all have common initial nodetype */

struct ast {
  enum node_types nodetype;
  struct ast *l;
  struct ast *r;
};

struct intval {
  enum node_types nodetype;			/* type N_integer */
  int number;
};

struct symref {
  enum node_types nodetype;			/* type N_symbol_ref */
  struct symbol *s;
};

struct symasgn {
  enum node_types nodetype;			/* type = N_assignment */
  struct symbol *s;
  struct ast *v;		/* value */
};

/* build an AST */
struct ast *newast(enum node_types nodetype, struct ast *l, struct ast *r);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newint(int d);

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* evaluate an AST */
double eval(struct ast *);

/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno; /* from lexer */
// void yyerror(char *s, ...);

extern int debug;
void dumpast(struct ast *a, int level);
