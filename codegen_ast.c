#include "llvm-c/Core.h"

#include "sprola.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(char const*);

/*----------------------------------------------------------------------------*/
void emit_x(LLVMModuleRef mod, struct ast *a)
{
  if (a == NULL) {
    return;
  }
}
