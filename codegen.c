#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "llvm-c/Target.h"

#include "ast.h"
#include "sprola.h"
#include "utils.h"

#include <bsd/string.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern void yyerror(char const*);

extern LLVMModuleRef emit_standard(struct ast *a);
extern void finish_descriptor(LLVMValueRef uri);

extern char current_filename[MAX_FILENAME_SIZE];   // read source from here
extern char plugin_name[MAX_FILENAME_SIZE];    // write .ll output here

extern struct symbol symbol_table[NHASH];
extern struct plugin_filenames names;

extern int verbose_flag;
extern int ll_flag;

int need_option_write = 0;
LLVMTypeRef void_ptr;
LLVMTypeRef float_ptr;
LLVMTypeRef struct_plugin;
LLVMTypeRef struct_lv2_descriptor;
LLVMTypeRef struct_lv2_feature;
LLVMValueRef uri;
LLVMValueRef global_descriptor;

// These functions are for the standard, in every LV2 plugin functions
LLVMValueRef FN_instantiate;
LLVMValueRef FN_connect_port;
LLVMValueRef FN_activate;
LLVMValueRef FN_run;
LLVMValueRef FN_deactivate;
LLVMValueRef FN_cleanup;
LLVMValueRef FN_extension_data;
LLVMValueRef FN_descriptor;

/*----------------------------------------------------------------------------*/
void emit_option(LLVMModuleRef mod, struct ast *a)
{
  size_t len;
  char *literal;

  if (verbose_flag) {
    fprintf(stderr, "emitting option...\n");
  }

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
      need_option_write = 1;
      // but for now, just declare a simple variable
      LLVMAddGlobal(mod, LLVMFloatType(), ((struct symref *) opt->target)->sym->name);
      break;
    case OPT_audio_output:
      need_option_write = 1;
      LLVMAddGlobal(mod, LLVMFloatType(), ((struct symref *) opt->target)->sym->name);
      break;
    case OPT_control_in:
      need_option_write = 1;
      LLVMAddGlobal(mod, LLVMFloatType(), ((struct symref *) opt->target)->sym->name);
      break;
    case OPT_control_out:
      need_option_write = 1;
      LLVMAddGlobal(mod, LLVMFloatType(), ((struct symref *) opt->target)->sym->name);
      break;
    case OPT_uri:
      len = strlen(((struct symref *) opt->target)->sym->name);
      literal = strndup((((struct symref *) opt->target)->sym->name)+1,len-2);
      strlcat(literal, "\00", 3);

      uri = LLVMAddGlobal(mod, LLVMArrayType(LLVMInt8Type(), len-1), "URI_string");

      LLVMSetUnnamedAddr(uri, 1);
      LLVMSetLinkage(uri, LLVMPrivateLinkage);
      LLVMSetGlobalConstant(uri, 1);

      // Initialize with string:
      LLVMSetInitializer(uri, LLVMConstString(literal, len-1, 1));
      free(literal);
      break;
  }
}

/*----------------------------------------------------------------------------*/
void emit_options(LLVMModuleRef mod, struct ast *a)
{
  if (verbose_flag) {
    fprintf(stderr, "emitting options...\n");
  }

  if (a == NULL) {
    return;
  }

  if (a->nodetype != N_options) {
    fprintf(stderr, "expecting an options list, found nodetype %d", a->nodetype);
    return;
  }

  if (a->l != NULL) {
    switch (a->l->nodetype) {
      case N_options:
        emit_options(mod, a->l);
        break;
      case N_option:
        emit_option(mod, a->l);
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
        emit_options(mod, a->r);
        break;
      case N_option:
        emit_option(mod, a->r);
        break;
      default:
        fprintf(stderr, "expecting an options or option, found %d\n",
          a->r->nodetype);
        return;
    }
  }
}

/*----------------------------------------------------------------------------*/
void emit_declarations(LLVMModuleRef mod, struct ast *a)
{
  if (verbose_flag) {
    fprintf(stderr, "emitting declarations...\n");
  }

  if (a == NULL) {
    return;
  }
}

/*----------------------------------------------------------------------------*/
void emit_func_def(LLVMModuleRef mod, struct ast *a)
{
  struct funcdef *fn = (struct funcdef *) a;
  LLVMValueRef return_value;
  LLVMTypeRef ret_type;
  LLVMValueRef func;
  LLVMBasicBlockRef entry;

  char *name;

  if (verbose_flag) {
    fprintf(stderr, "emitting function definition...\n");
  }

  if (fn == NULL) {
    return;
  }

  if (fn->nodetype != N_func_def) {
    fprintf(stderr, "expecting function def\n");
    return;
  }

  name = fn->name->sym->name;
  return_value = NULL;

  LLVMBuilderRef builder = LLVMCreateBuilder();

  if (strcmp(name, "run") == 0) {
  } else if (strcmp(name, "instantiate") == 0) {
  } else if (strcmp(name, "activate") == 0) {
  } else if (strcmp(name, "deactivate") == 0) {
  } else if (strcmp(name, "cleanup") == 0) {
  } else if (strcmp(name, "extension_data") == 0) {
  } else if (strcmp(name, "connect_port") == 0) {
  } else {
    LLVMTypeRef param_types[] = { LLVMInt8Type(), LLVMInt32Type()};
    ret_type = LLVMFunctionType(LLVMVoidType(), param_types, 2, 0);

    func = LLVMAddFunction(mod, name, ret_type);
    entry = LLVMAppendBasicBlock(func, "entry");

    LLVMPositionBuilderAtEnd(builder, entry);
    LLVMBuildRet(builder, return_value);
  }

  LLVMDisposeBuilder(builder);
}

/*----------------------------------------------------------------------------*/
void emit_functions(LLVMModuleRef mod, struct ast *a)
{
  if (verbose_flag) {
    fprintf(stderr, "emitting functions...\n");
  }

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
        emit_functions(mod, a->l);
        break;
      case N_func_def:
        emit_func_def(mod, a->l);
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
        emit_functions(mod, a->r);
        break;
      case N_func_def:
        emit_func_def(mod, a->r);
        break;
      default:
        fprintf(stderr, "expecting functions or function, found %d\n",
          a->r->nodetype);
        return;
    }
  }
}

/*----------------------------------------------------------------------------*/
void emit_code(struct ast *a)
{
  char *error = NULL;

  if (verbose_flag) {
    fprintf(stderr, "emitting code...\n");
  }

  if (a->nodetype != N_program) {
    fprintf(stderr, "error: top level ast node is not a program\n");
    fprintf(stderr, "nodetype is %d\n", a->nodetype);
    return;
  }

  // Do the setup that's standard
  LLVMInitializeNativeTarget();
  LLVMModuleRef mod = emit_standard(a);

  // Do the rest
  emit_options(mod, ((struct prog *) a)->opts);
  emit_declarations(mod, ((struct prog *) a)->decls);
  emit_functions(mod, ((struct prog *) a)->funcs);

  finish_descriptor(uri);

  // It's built, lets verify
  fprintf(stderr, "verifying generated bit code...\n");
  LLVMVerifyModule(mod, LLVMPrintMessageAction, &error);
  LLVMDisposeMessage(error);
  fprintf(stderr, "verify complete\n");


  generate_obj_lib(mod, &names);
  emit_plugin_ttl(mod, a, &names);
  emit_manifest_ttl(mod, a, &names);

  LLVMDisposeModule(mod);
  LLVMShutdown();

}
