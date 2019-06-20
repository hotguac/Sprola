#include "llvm-c/Core.h"

#include "sprola.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void yyerror(char const*);

extern char current_filename[MAX_FILENAME_SIZE];   // read source from here

extern struct symbol symbol_table[NHASH];

/*----------------------------------------------------------------------------*/
void emit_x(LLVMModuleRef mod, struct ast *a)
{
  if (a == NULL) {
    return;
  }
}
