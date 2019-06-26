/* Signal PROcessing LAnguage */
#ifndef __SPROLA__
#define __SPROLA__

#include <stdio.h>

#include "llvm-c/Core.h"

#include "ast.h"
#include "symbols.h"
#include "utils.h"

struct symbol *lookup(char* id);
void addref(int lineno, char* filename, char* id, int flags);
void emit_manifest_ttl(LLVMModuleRef mod, struct ast *a, struct plugin_filenames *names);
void emit_plugin_ttl(LLVMModuleRef mod, struct ast *a, struct plugin_filenames *names);

extern char current_filename[MAX_FILENAME_SIZE];   // read source from here
extern int verbose_flag;

extern int ll_flag;

/* These external routines are defined in or generated from sprola.l */
extern int yylineno; /* from lexer */
extern int yylex();
extern int yyparse();
extern FILE * yyin;
extern char *yytext;

#endif
