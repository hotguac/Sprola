#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "llvm-c/Target.h"

#include "ast.h"
#include "codegen.h"
#include "sprola.h"
#include "utils.h"

#include <bsd/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

LLVMValueRef uri;

/*----------------------------------------------------------------------------*/
// audio and control ports are handled in codegen_std
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
      //TODO(future) - right now we only handle LV2 format, add logic to explicity chose LV2
      break;
    case OPT_audio_input:
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
// all global declarations are handled in the codegen_std and added to the
// plugin structure saved between calls of run()
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

//----------------------------------------------------------------------------
// This is a user defined function
//----------------------------------------------------------------------------
void emit_new_function_def(LLVMModuleRef mod, struct ast *a) {

}

//----------------------------------------------------------------------------
// This adds user code to the end of the standard function 'run'
//----------------------------------------------------------------------------
void add_to_run_function(LLVMModuleRef mod, struct ast *a) {
  LLVMValueRef return_value;
  LLVMValueRef func;
  LLVMBasicBlockRef entry_block;
  LLVMBasicBlockRef user_block;
  LLVMBasicBlockRef exit_block;

  char *name;

  struct funcdef *fn = (struct funcdef *) a;

  if (verbose_flag) {
    fprintf(stderr, "adding to 'run' function definition...\n");
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

  func = LLVMGetNamedFunction(mod, name);

  entry_block = LLVMGetFirstBasicBlock(func);
  if (strcmp(LLVMGetBasicBlockName(entry_block), "entry") != 0) {
    fprintf(stderr, "Error - expected entry block\n");
    return;
  }

  user_block = LLVMGetNextBasicBlock(entry_block);
  if (strcmp(LLVMGetBasicBlockName(user_block), "user") != 0) {
    fprintf(stderr, "Error - expected user block\n");
    return;
  }

  exit_block = LLVMGetNextBasicBlock(user_block);
  if (strcmp(LLVMGetBasicBlockName(exit_block), "exit") != 0) {
    fprintf(stderr, "Error - expected exit block\n");
    return;
  }

  LLVMValueRef term = LLVMGetBasicBlockTerminator(user_block);
  if (term == NULL) {
    fprintf(stderr, "Error - expected user block terminator\n");
  }

  LLVMPositionBuilderBefore(builder, term);

  // User code goes here, in the user block before the terminator statement
  if (a != NULL) {
    // then we have user code, so remove the standard branch to exit block
    LLVMInstructionEraseFromParent(term);
    LLVMPositionBuilderAtEnd(builder, user_block); // needed to re-establish position
  }

  emit_x(builder, a);
}

/*----------------------------------------------------------------------------*/
void add_to_function_def(LLVMModuleRef mod, struct ast *a)
{

}

/*----------------------------------------------------------------------------*/
void emit_func_def(LLVMModuleRef mod, struct ast *a)
{
  struct funcdef *fn = (struct funcdef *) a;
  LLVMValueRef return_value;
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
    add_to_run_function(mod,a);
  } else if (strcmp(name, "instantiate") == 0) {
    add_to_function_def(mod,a);
  } else if (strcmp(name, "activate") == 0) {
    add_to_function_def(mod,a);
  } else if (strcmp(name, "deactivate") == 0) {
    add_to_function_def(mod,a);
  } else if (strcmp(name, "cleanup") == 0) {
    add_to_function_def(mod,a);
  } else if (strcmp(name, "extension_data") == 0) {
    add_to_function_def(mod,a);
  } else if (strcmp(name, "connect_port") == 0) {
    add_to_function_def(mod,a);
  } else {
    emit_new_function_def(mod, a);
  }

  LLVMDisposeBuilder(builder);
}

/*----------------------------------------------------------------------------*/
void emit_functions(LLVMModuleRef mod, struct ast *a)
{
  if (verbose_flag) {
    //fprintf(stderr, "emitting functions...\n");
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

  // Do the setup that's standard to all programs
  LLVMInitializeNativeTarget();

  info = get_port_info(a);
  LLVMModuleRef mod = emit_standard(a);

  // Do the rest
  emit_options(mod, ((struct prog *) a)->opts);
  emit_declarations(mod, ((struct prog *) a)->decls);
  emit_functions(mod, ((struct prog *) a)->funcs);

  finish_descriptor(uri);

  if (verbose_flag) {
    fprintf(stderr, "writing module to verbose_dump.ll\n");
    LLVMPrintModuleToFile(mod, "verbose_dump.ll", &error);
    LLVMDisposeMessage(error);
  }

  // It's built, lets verify
  fprintf(stderr, "verifying generated bit code...\n");
  //LLVMVerifyModule(mod, LLVMPrintMessageAction, &error);
  LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);
  fprintf(stderr, "verify complete, generating object library...\n");


  generate_obj_lib(mod, &names);
  fprintf(stderr, "generating ttl files\n");
  emit_plugin_ttl(mod, a, &names);
  emit_manifest_ttl(mod, a, &names);
  fprintf(stderr, "cleaning up...\n");

  LLVMDisposeModule(mod);
  LLVMShutdown();
  fprintf(stderr, "finished!\n");
}
