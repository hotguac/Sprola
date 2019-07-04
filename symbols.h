#ifndef __SYMBOLS__
#define __SYMBOLS__

#include <llvm-c/Core.h>

#define NHASH 6151

enum symbol_family {FUNCTION_NAME,
                  VARIABLE_NAME,
                  MEMBER_NAME,
                  INPUT_AUDIO_NAME,
                  OUTPUT_AUDIO_NAME,
                  INPUT_CONTROL_NAME,
                  OUTPUT_CONTROL_NAME
                };

struct symbol {
  char *name;
  enum symbol_family sym_family;
  char *sym_typ;
  struct ast *func;           // An AST for a function declaration
  struct symlist *syms;       // Function parameter list
  struct ref *reflist;        // Where used
  LLVMValueRef value;         // pointer to llvm stack location (LLVMBuildAlloca())
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

extern struct symbol symbol_table[NHASH];

struct symbol *lookup(char* id);
void addref(int lineno, char* filename, char* id, int flags);

#endif
