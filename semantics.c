#include "sprola.h"

#include <bsd/string.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llvm-c/Core.h"

void yyerror(char const*);

/*----------------------------------------------------------------------------*/
void check_option(LLVMModuleRef mod, struct ast *a)
{
  int len;
  char *literal;
  char *lit;

  if (a == NULL) {
    return;
  }

  if (a->nodetype != N_option) {
    fprintf(stderr, "expecting an option, found nodetype %d", a->nodetype);
    return;
  }

  struct setopt *opt = ((struct setopt *) a);

  switch (opt->option_flag) {
    case OPT_lv2:
      break;
    case OPT_audio_input:
      // save up in's and out's, plus other global declarations
      // into a table, set flag, and then when we hit the first function
      // write IR later using LLVMStructType()
      break;
    case OPT_audio_output:
      break;
    case OPT_control_in:
      break;
    case OPT_control_out:
      break;
    case OPT_uri:
      len = strlen(((struct symref *) opt->target)->sym->name);
      literal = strndup((((struct symref *) opt->target)->sym->name)+1,len-2);
      strlcat(literal, "\00", 3);
      free(literal);
      break;
  }
}

/*----------------------------------------------------------------------------*/
void check_options(LLVMModuleRef mod, struct ast *a)
{
  if (a == NULL) {
    return;
  }

  if (a->nodetype != N_options) {
    fprintf(stderr, "expecting an options list, found nodetype %d",
      a->nodetype);
    return;
  }

  if (a->l != NULL) {
    switch (a->l->nodetype) {
      case N_options:
        check_options(mod, a->l);
        break;
      case N_option:
        check_option(mod, a->l);
        break;
      default:
        fprintf(stderr, "expecting an options or option, found %d\n",
          a->l->nodetype);
        return;
    }
  }

  if (a->r != NULL) {
    switch (a->r->nodetype) {
      case N_options:
        check_options(mod, a->r);
        break;
      case N_option:
        check_option(mod, a->r);
        break;
      default:
        fprintf(stderr, "expecting an options or option, found %d\n",
          a->r->nodetype);
        return;
    }
  }
}

/*----------------------------------------------------------------------------*/
void check_declarations(LLVMModuleRef mod, struct ast *a)
{
  if (a == NULL) {
    return;
  }
}

/*----------------------------------------------------------------------------*/
void check_func_def(LLVMModuleRef mod, struct ast *a)
{
  struct funcdef *fn = (struct funcdef *) a;

  char *name;

  if (fn == NULL) {
    return;
  }

  if (fn->nodetype != N_func_def) {
    fprintf(stderr, "expecting function def\n");
    return;
  }

  name = fn->name->sym->name;

  if (strcmp(name, "run") == 0) {
  } else if (strcmp(name, "instantiate") == 0) {
  } else if (strcmp(name, "activate") == 0) {
  } else if (strcmp(name, "deactivate") == 0) {
  } else if (strcmp(name, "cleanup") == 0) {
  } else if (strcmp(name, "extension_data") == 0) {
  } else if (strcmp(name, "connect_port") == 0) {
  } else {
  }

}

/*----------------------------------------------------------------------------*/
void check_functions(LLVMModuleRef mod, struct ast *a)
{
  if (a == NULL) {
    return;
  }

  if (a->nodetype != N_functions) {
    fprintf(stderr, "expecting an functions list, found nodetype %d",
      a->nodetype);
    return;
  }

  if (a->l != NULL) {
    switch (a->l->nodetype) {
      case N_functions:
        check_functions(mod, a->l);
        break;
      case N_func_def:
        check_func_def(mod, a->l);
        break;
      default:
        fprintf(stderr, "expecting functions or function, found %d\n",
          a->l->nodetype);
        return;
    }
  }

  if (a->r != NULL) {
    switch (a->r->nodetype) {
      case N_functions:
        check_functions(mod, a->r);
        break;
      case N_func_def:
        check_func_def(mod, a->r);
        break;
      default:
        fprintf(stderr, "expecting functions or function, found %d\n",
          a->r->nodetype);
        return;
    }
  }
}

/*----------------------------------------------------------------------------*/
void check_code(LLVMModuleRef mod, struct ast *a)
{
  char *error = NULL;

  if (a != NULL) {
    if (a->nodetype != N_program) {
      fprintf(stderr, "error: top level ast node is not a program\n");
      fprintf(stderr, "nodetype is %d\n", a->nodetype);
      return;
    }
  // Do the rest
  check_options(mod, ((struct prog *) a)->opts);
  check_declarations(mod, ((struct prog *) a)->decls);
  check_functions(mod, ((struct prog *) a)->funcs);
  }

}
