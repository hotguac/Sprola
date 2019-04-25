#ifndef __SYMBOLS__
#define __SYMBOLS__

#define NHASH 17

enum symbol_type {FUNCTION_NAME, VARIABLE_NAME, MEMBER_NAME};

struct symbol {
  char *name;
  enum symbol_type sym_type;
  struct ast *func;           // An AST for a function declaration
  struct symlist *syms;       // Function parameter list
  struct ref *reflist;        // Where used
};

/* list of symbols, for an argument list */
struct symlist {
  struct symbol *sym;
  struct symlist *next;
};

struct ref {
  struct ref *next;
  char *filename;
  int flags;
  int lineno;
};


#endif
