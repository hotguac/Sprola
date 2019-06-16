#include "llvm-c/Core.h"
#include "llvm-c/Types.h"
#include "llvm-c/Analysis.h"
#include "llvm-c/BitWriter.h"
#include "llvm-c/Target.h"

#include "ast.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

extern void yyerror(char const*);

extern char current_filename[MAX_FILENAME_SIZE];   // read source from here
//extern char output_filename[MAX_FILENAME_SIZE];    // write .ll output here

extern struct symbol symbol_table[NHASH];

/*----------------------------------------------------------------------------*/
void emit_x(LLVMModuleRef mod, struct ast *a)
{
  if (a == NULL) {
    return;
  }
}
