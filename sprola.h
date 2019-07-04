/* Signal PROcessing LAnguage */
#ifndef __SPROLA__
#define __SPROLA__

#include <stdio.h>

#include "llvm-c/Core.h"

#include "ast.h"
#include "symbols.h"
#include "utils.h"

extern char current_filename[MAX_FILENAME_SIZE];   // read source from here

extern int ll_flag;
extern int trace_flag;
extern int verbose_flag;

/* These external routines are defined in or generated from sprola.l */
extern int yylineno; /* from lexer */
extern int yylex();
extern int yyparse();
extern FILE * yyin;
extern char *yytext;

#endif
