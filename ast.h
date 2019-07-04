/* Includes Source code from "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1 Copyright (c) 2009, Taughannock Networks.
*/
#ifndef AST_H
#define AST_H

enum node_types {
  N_program = 1,
  N_functions,
  N_func_def,
  N_options,
  N_option,
  N_var_declarations,
  N_var_declaration,
  N_assignment,
  N_condition,
  N_integer,
  N_float,
  N_symbol_ref,
  N_array_ref,
  N_statement_list,
  N_for_loop,
  N_while_loop,
  N_until_loop,
  N_add,
  N_subtract,
  N_multiply,
  N_divide,
  N_scope,
  N_negate
};

enum bifs {			/* built-in functions */
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print
};

enum option_flags {
  OPT_lv2 = 1,
  OPT_audio_input,
  OPT_audio_output,
  OPT_control_in,
  OPT_control_out,
  OPT_uri
};

enum logical_types {
  L_less_than = 1,
  L_greater_than,
  L_equals
};

enum return_types {
  TY_void = 1,
  TY_integer,
  TY_float
};

/* nodes in the Abstract Syntax Tree */
/* all have common initial nodetype */


struct ast {
  enum node_types nodetype;
  struct ast *l;
  struct ast *r;
};

struct setopt {
  enum node_types nodetype;     /* type N_option */
  enum option_flags option_flag; /* which option */
  struct ast *target;           /* symbol        */
};

struct intval {
  enum node_types nodetype;			/* type N_integer */
  int number;
};

struct floatval {
  enum node_types nodetype;			/* type N_float */
  float number;
};

struct symref {
  enum node_types nodetype;			/* type N_symbol_ref */
  struct symbol *sym;
};

struct var_decl {
  enum node_types nodetype;			/* type N_symbol_ref */
  struct ast *sym;
};

struct symasgn {
  enum node_types nodetype;			/* type = N_assignment */
  //struct symbol *sym;           /* symbol */
  struct ast *target;
  struct ast *value;		        /* expression */
};

struct arrayref {
  enum node_types nodetype;			/* type N_symbol_ref */
  struct symbol *sym;           /* symbol        */
  struct ast *index;            /* expression */
};

struct condition {
  enum node_types nodetype;
  struct ast *left;
  enum logical_types operator;
  struct ast *right;
};

struct forloop {
  enum node_types nodetype;			/* type N_for_loop */
  struct ast *initialize;       /* expression */
  struct ast *condition;        /* expression */
  struct ast *post;             /* expression */
  struct ast *codeblock;        /* statements */
};

struct prog {
  enum node_types nodetype;			/* type N_for_loop */
  struct ast *opts;       /* options */
  struct ast *decls;      /* variable declarations */
  struct ast *funcs;      /* function definitions */
};

struct funcdef {
  enum node_types nodetype;			/* type N_func_def */
  enum return_types return_type;
  struct ast *arglist;
  struct symref *name;
  struct ast *body;
};

/* build an AST */
struct ast *newprogram(struct ast *options, struct ast *decls, struct ast *funcs);
struct ast *newast(enum node_types nodetype, struct ast *l, struct ast *r);
struct ast *newasgn(struct ast *target, struct ast *value);
struct ast *newarrayref(struct symbol *s, struct ast *v);
struct ast *newint(int d);
struct ast *newfloat(float f);
struct ast *newvardecl(char *type, struct ast *s);
struct ast *newsymref(struct symbol *s);
struct ast *newforloop(struct ast *init, struct ast *cond, struct ast *post, struct ast *block);
struct ast *newlessthan(struct ast *left, struct ast *right);
struct ast *newfunction(int return_type, struct ast* name, struct ast* code);
struct ast *newoption(enum option_flags flag, struct ast* sym);

/* delete and free an AST */
void treefree(struct ast * a);

void dumpast(struct ast *a, int level);

#endif
