#include "llvm-c/Core.h"
#include "llvm-c/Types.h"
#include "llvm-c/Analysis.h"
#include "llvm-c/BitWriter.h"
#include "llvm-c/Target.h"

#include "ast.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

extern void yyerror(char const*);
extern char *current_filename;   // read source from here
extern char *output_filename;    // write .ll output here
extern struct symbol symtab[NHASH];

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
      lit = strcat(literal, "\00");

      uri = LLVMAddGlobal(mod, LLVMArrayType(LLVMInt8Type(), len-1), "URI_string");

      LLVMSetUnnamedAddr(uri, 1);
      LLVMSetLinkage(uri, LLVMPrivateLinkage);
      LLVMSetGlobalConstant(uri, 1);

      // Initialize with string:
      LLVMSetInitializer(uri, LLVMConstString(lit, len-1, 1));
      break;
  }
}

/*----------------------------------------------------------------------------*/
void emit_options(LLVMModuleRef mod, struct ast *a)
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
          a->l->nodetype);
        return;
    }
  }
}

/*----------------------------------------------------------------------------*/
void emit_lv2_descriptor(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the descriptor function, the body statements
  //  are added here because right now we only allow a single
  //  plugin per Sprola module
  // -------
  LLVMTypeRef ret_type =
    LLVMFunctionType(LLVMPointerType(struct_lv2_descriptor, 0),
      (LLVMTypeRef []) {
        LLVMInt32Type()
      }, 1, 0);

  FN_descriptor = LLVMAddFunction(mod, "lv2_descriptor", ret_type);
  LLVMSetValueName(LLVMGetParam(FN_descriptor, 0), "index");
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_descriptor, "");

  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef descr = LLVMBuildAlloca(builder,
    LLVMPointerType(struct_lv2_descriptor, 0), "");
  LLVMSetAlignment(descr, 8);

  LLVMValueRef x = LLVMBuildAlloca(builder, LLVMInt32Type(), "");
  LLVMSetAlignment(x, 4);

  LLVMSetAlignment(
    LLVMBuildStore(builder, LLVMGetParam(FN_descriptor, 0), x), 4);

  LLVMValueRef y = LLVMBuildLoad(builder, x, "");
  LLVMSetAlignment(y, 4);

  // Set up a switch statement by first
  //  setting up the case basic blocks
  LLVMBasicBlockRef case0 = LLVMAppendBasicBlock(FN_descriptor, "");
  LLVMBasicBlockRef case_default = LLVMAppendBasicBlock(FN_descriptor, "");
  LLVMBasicBlockRef sw_end = LLVMAppendBasicBlock(FN_descriptor, "");

  // Now define the switch statement
  LLVMValueRef sw = LLVMBuildSwitch(builder, y, case_default, 1);

  // Build the case 0 basic block
  LLVMAddCase(sw, LLVMConstInt(LLVMInt32Type(), 0, 0), case0);
  LLVMPositionBuilderAtEnd(builder, case0);

  global_descriptor = LLVMAddGlobal(mod, struct_lv2_descriptor, "descriptor");
  LLVMSetLinkage(global_descriptor, LLVMInternalLinkage);

  LLVMSetAlignment(LLVMBuildStore(builder,global_descriptor, descr),8);
  LLVMBuildBr(builder, sw_end);

  // Build the default basic block
  LLVMPositionBuilderAtEnd(builder, case_default);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMConstNull(LLVMPointerType(struct_lv2_descriptor, 0)), descr), 8);
  LLVMBuildBr(builder, sw_end);

  // Build the after the switch basic block
  LLVMPositionBuilderAtEnd(builder, sw_end);

  LLVMValueRef return_value = LLVMBuildLoad(builder, descr, "");
  LLVMSetAlignment(return_value, 8);
  LLVMBuildRet(builder, return_value);
}

/*----------------------------------------------------------------------------*/
void emit_instantiate(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the stock instantiate function,
  // the statements from the Sprola source will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(void_ptr,
    (LLVMTypeRef []) {
      LLVMPointerType(struct_lv2_descriptor, 0),
      LLVMDoubleType(),
      LLVMPointerType(LLVMInt8Type(), 0),
      LLVMPointerType(LLVMPointerType(struct_lv2_feature, 0), 0)
    }, 4, 0);

  FN_instantiate = LLVMAddFunction(mod, "instantiate", ret_type);
  LLVMSetValueName(LLVMGetParam(FN_instantiate, 0), "descriptor");
  LLVMSetValueName(LLVMGetParam(FN_instantiate, 1), "rate");
  LLVMSetValueName(LLVMGetParam(FN_instantiate, 2), "bundle_path");
  LLVMSetValueName(LLVMGetParam(FN_instantiate, 3), "features");

  LLVMSetLinkage(FN_instantiate, LLVMInternalLinkage);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_instantiate, "");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef descr = LLVMBuildAlloca(builder, LLVMPointerType(struct_lv2_descriptor, 0), "");
  LLVMValueRef rate = LLVMBuildAlloca(builder, LLVMDoubleType(), "");
  LLVMValueRef bundle_path = LLVMBuildAlloca(builder, void_ptr, "");
  LLVMValueRef features = LLVMBuildAlloca(builder, LLVMPointerType(LLVMPointerType(struct_lv2_feature, 0),0), "");
  LLVMValueRef plugin = LLVMBuildAlloca(builder, LLVMPointerType(struct_plugin, 0), "plugin");

  LLVMSetAlignment(descr, 8);
  LLVMSetAlignment(rate, 8);
  LLVMSetAlignment(bundle_path, 8);
  LLVMSetAlignment(features, 8);
  LLVMSetAlignment(plugin, 8);

  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_instantiate, 0), descr), 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_instantiate, 1), rate), 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_instantiate, 2), bundle_path), 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_instantiate, 3), features), 8);

  //TODO: add noalias : Define external function malloc
  LLVMTypeRef malloc_type = LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0),
    (LLVMTypeRef []) { LLVMInt64Type()}, 1, 0);
  LLVMValueRef ext_malloc = LLVMAddFunction(mod, "malloc", malloc_type);
  LLVMSetLinkage(ext_malloc, LLVMExternalLinkage);

  //TODO: build the size, i.e. 24, programatically instead of hard code
  LLVMValueRef args[] = { LLVMConstInt(LLVMInt64Type(), 24, 0) };
  LLVMValueRef result = LLVMBuildCall(builder, ext_malloc, args, 1, "");

  LLVMValueRef cast = LLVMBuildBitCast(builder, result, LLVMPointerType(struct_plugin, 0), "");
  LLVMSetAlignment(LLVMBuildStore(builder, cast, plugin), 8);

  LLVMValueRef y = LLVMBuildLoad(builder, plugin, "");
  LLVMSetAlignment(y, 8);

  LLVMValueRef ret_val = LLVMBuildBitCast(builder, y, LLVMPointerType(LLVMInt8Type(), 0), "");

  LLVMBuildRet(builder, ret_val);
}

/*----------------------------------------------------------------------------*/
void emit_connect_port(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the connect_port function, the body statements will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(),
    (LLVMTypeRef []){
      void_ptr,
      LLVMInt32Type(),
      void_ptr
    }, 3, 0);

  FN_connect_port = LLVMAddFunction(mod, "connect_port", ret_type);
  LLVMSetLinkage(FN_connect_port, LLVMInternalLinkage);

  LLVMSetValueName(LLVMGetParam(FN_connect_port, 0), "instance");
  LLVMSetValueName(LLVMGetParam(FN_connect_port, 1), "port");
  LLVMSetValueName(LLVMGetParam(FN_connect_port, 2), "data");

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_connect_port, "");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef instance = LLVMBuildAlloca(builder, void_ptr, "");
  LLVMValueRef port = LLVMBuildAlloca(builder, LLVMInt32Type(), "");
  LLVMValueRef data = LLVMBuildAlloca(builder, void_ptr, "");
  LLVMValueRef plugin = LLVMBuildAlloca(builder, LLVMPointerType(struct_plugin, 0), "plugin");

  LLVMSetAlignment(instance, 8);
  LLVMSetAlignment(port, 4);
  LLVMSetAlignment(data, 8);
  LLVMSetAlignment(plugin, 8);

  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_connect_port, 0), instance), 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_connect_port, 1), port), 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_connect_port, 2), data), 8);

  LLVMValueRef y = LLVMBuildLoad(builder, instance, "");
  LLVMSetAlignment(y, 8);

  LLVMValueRef z = LLVMBuildBitCast(builder, y, LLVMPointerType(struct_plugin, 0), "");
  LLVMSetAlignment(LLVMBuildStore(builder, z, plugin), 8);

  LLVMValueRef index = LLVMBuildLoad(builder, port, "");
  LLVMSetAlignment(index, 4);

  //***********************************************************************
  // Set up switch statement blocks
  //TODO: the number of cases should be built based on number of option ports
  //***********************************************************************
  LLVMBasicBlockRef case0 = LLVMAppendBasicBlock(FN_connect_port, "");
  LLVMBasicBlockRef case1 = LLVMAppendBasicBlock(FN_connect_port, "");
  LLVMBasicBlockRef case2 = LLVMAppendBasicBlock(FN_connect_port, "");
  LLVMBasicBlockRef sw_end = LLVMAppendBasicBlock(FN_connect_port, "");

  // Define switch statement
  LLVMValueRef sw = LLVMBuildSwitch(builder, index, sw_end, 3);

  //***********************************************************************
  //  Case 0
  //***********************************************************************
  LLVMAddCase(sw, LLVMConstInt(LLVMInt32Type(), 0, 0), case0);
  LLVMPositionBuilderAtEnd(builder, case0);

  // Add body of case here
  LLVMValueRef dat = LLVMBuildLoad(builder, data, "");
  LLVMSetAlignment(dat, 8);

  LLVMValueRef dat_array = LLVMBuildBitCast(builder, dat, LLVMPointerType(LLVMFloatType(), 0), "");

  LLVMValueRef plug = LLVMBuildLoad(builder, plugin, "");
  LLVMSetAlignment(plug, 8);

  LLVMValueRef tt = LLVMBuildStructGEP2(builder, struct_plugin, plug, 0, "");
  LLVMSetAlignment(LLVMBuildStore(builder, dat_array, tt), 8);

  LLVMBuildBr(builder, sw_end);

  //***********************************************************************
  // Case 1
  //***********************************************************************
  LLVMAddCase(sw, LLVMConstInt(LLVMInt32Type(), 1, 0), case1);
  LLVMPositionBuilderAtEnd(builder, case1);

  dat = LLVMBuildLoad(builder, data, "");
  LLVMSetAlignment(dat, 8);

  dat_array = LLVMBuildBitCast(builder, dat, LLVMPointerType(LLVMFloatType(), 0), "");

  plug = LLVMBuildLoad(builder, plugin, "");
  LLVMSetAlignment(plug, 8);

  tt = LLVMBuildStructGEP2(builder, struct_plugin, plug, 1, "");
  LLVMSetAlignment(LLVMBuildStore(builder, dat_array, tt), 8);

  LLVMBuildBr(builder, sw_end);

  //***********************************************************************
  // Case 2
  //***********************************************************************
  LLVMAddCase(sw, LLVMConstInt(LLVMInt32Type(), 2, 0), case2);
  LLVMPositionBuilderAtEnd(builder, case2);

  dat = LLVMBuildLoad(builder, data, "");
  LLVMSetAlignment(dat, 8);

  dat_array = LLVMBuildBitCast(builder, dat, LLVMPointerType(LLVMFloatType(), 0), "");

  plug = LLVMBuildLoad(builder, plugin, "");
  LLVMSetAlignment(plug, 8);

  LLVMValueRef tt2 = LLVMBuildStructGEP2(builder, struct_plugin, plug, 2, "");
  LLVMSetAlignment(LLVMBuildStore(builder, dat_array, tt2), 8);

  LLVMBuildBr(builder, sw_end);

  //***********************************************************************
  // Switch end
  //***********************************************************************
  LLVMPositionBuilderAtEnd(builder, sw_end);

  LLVMBuildRet(builder, NULL);
}

/*----------------------------------------------------------------------------*/
void emit_activate(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the activate function, the body statements will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(),
    (LLVMTypeRef []){
      void_ptr
    }, 1, 0);

  FN_activate = LLVMAddFunction(mod, "activate", ret_type);
  LLVMSetLinkage(FN_activate, LLVMInternalLinkage);

  LLVMSetValueName(LLVMGetParam(FN_activate, 0), "instance");

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_activate, "");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef instance = LLVMBuildAlloca(builder, void_ptr, "");
  LLVMSetAlignment(instance, 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_activate, 0), instance), 8);

  LLVMBuildRet(builder, NULL);
}

/*----------------------------------------------------------------------------*/
void emit_run(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the run function, the body statements will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(),
    (LLVMTypeRef []){
      void_ptr,
      LLVMInt32Type()
    }, 2, 0);

  FN_run = LLVMAddFunction(mod, "run", ret_type);
  LLVMSetValueName(LLVMGetParam(FN_run, 0), "instance");
  LLVMSetValueName(LLVMGetParam(FN_run, 1), "n_samples");

  LLVMSetLinkage(FN_run, LLVMInternalLinkage);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_run, "");

  LLVMPositionBuilderAtEnd(builder, entry);
  LLVMBuildRet(builder, NULL);
}

/*----------------------------------------------------------------------------*/
void emit_deactivate(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the deactivate function, the body statements will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(),
    (LLVMTypeRef []){
      void_ptr
    }, 1, 0);

  FN_deactivate = LLVMAddFunction(mod, "deactivate", ret_type);
  LLVMSetValueName(LLVMGetParam(FN_deactivate, 0), "instance");

  LLVMSetLinkage(FN_deactivate, LLVMInternalLinkage);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_deactivate, "");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef instance = LLVMBuildAlloca(builder, void_ptr, "");
  LLVMSetAlignment(instance, 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_deactivate, 0), instance), 8);

  LLVMBuildRet(builder, NULL);
}

/*----------------------------------------------------------------------------*/
void emit_cleanup(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the cleanup function, the body statements will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef []) { void_ptr }, 1, 0);

  FN_cleanup = LLVMAddFunction(mod, "cleanup", ret_type);
  LLVMSetValueName(LLVMGetParam(FN_cleanup, 0), "instance");
  LLVMSetLinkage(FN_cleanup, LLVMInternalLinkage);

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_cleanup, "");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef instance = LLVMBuildAlloca(builder, void_ptr, "");
  LLVMSetAlignment(instance, 8);
  LLVMValueRef x = LLVMBuildStore(builder, LLVMGetParam(FN_cleanup, 0), instance);
  LLVMSetAlignment(x, 8);

  LLVMValueRef y = LLVMBuildLoad(builder, instance, "");
  LLVMSetAlignment(y, 8);

  LLVMTypeRef free_type = LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef []){ void_ptr }, 1, 0);
  LLVMValueRef ext_free = LLVMAddFunction(mod, "free", free_type);
  LLVMSetLinkage(ext_free, LLVMExternalLinkage);

  LLVMValueRef args[] = { y };
  LLVMValueRef result = LLVMBuildCall(builder, ext_free, args, 1, "");

  LLVMBuildRet(builder, NULL);
}

/*----------------------------------------------------------------------------*/
void emit_extension_data(LLVMModuleRef mod, LLVMBuilderRef builder)
{
  // -------
  // define the extension_data function, the body statements will be added later
  // -------
  LLVMTypeRef ret_type = LLVMFunctionType(void_ptr,
    (LLVMTypeRef []){
      void_ptr
    }, 1, 0);

  FN_extension_data = LLVMAddFunction(mod, "extension_data", ret_type);
  LLVMSetLinkage(FN_extension_data, LLVMInternalLinkage);

  LLVMSetValueName(LLVMGetParam(FN_extension_data, 0), "uri");

  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(FN_extension_data, "");
  LLVMPositionBuilderAtEnd(builder, entry);

  LLVMValueRef x = LLVMBuildAlloca(builder, LLVMPointerType(LLVMInt8Type(), 0), "");
  LLVMSetAlignment(x, 8);

  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_extension_data, 0), x),8);
  LLVMBuildRet(builder, LLVMConstNull(void_ptr));
}

/*----------------------------------------------------------------------------*/
LLVMModuleRef emit_standard(void)
{
  void_ptr = LLVMPointerType(LLVMInt8Type(), 0);
  float_ptr = LLVMPointerType(LLVMFloatType(), 0);

  LLVMContextRef global = LLVMGetGlobalContext();
  LLVMModuleRef mod = LLVMModuleCreateWithNameInContext(current_filename, global);

  // -------
  // This is defined by the LV2 specification
  struct_lv2_feature = LLVMStructCreateNamed(global, "struct._LV2_Feature");

  LLVMTypeRef lv2_feature_body[] = { void_ptr, void_ptr };
  LLVMStructSetBody(struct_lv2_feature, lv2_feature_body, 2, 0);

  // -------
  // This is defined by the LV2 specification, we'll fill body
  // after the functions created
  // -------
  struct_lv2_descriptor = LLVMStructCreateNamed(global, "struct._LV2_Descriptor");

  // -------
  // This is the structure that holds all the global fields, we'll fill
  // the body after we've processed all the options and global declares
  // -------
  struct_plugin = LLVMStructCreateNamed(global, "struct.Plugin");
  LLVMStructSetBody(struct_plugin,
    (LLVMTypeRef []) {
      LLVMPointerType(LLVMFloatType(), 0),
      LLVMPointerType(LLVMFloatType(), 0),
      LLVMPointerType(LLVMFloatType(), 0)
    }, 3, 0);

  LLVMBuilderRef builder = LLVMCreateBuilderInContext(global);

  emit_lv2_descriptor(mod, builder);
  emit_instantiate(mod, builder);
  emit_connect_port(mod, builder);
  emit_activate(mod, builder);
  emit_run(mod, builder);
  emit_deactivate(mod, builder);
  emit_cleanup(mod, builder);
  emit_extension_data(mod, builder);

  LLVMDisposeBuilder(builder);

  return(mod);
}

/*----------------------------------------------------------------------------*/
void finish_descriptor()
{
  LLVMStructSetBody(struct_lv2_descriptor, (LLVMTypeRef []) {
    void_ptr,
    LLVMTypeOf(FN_instantiate),
    LLVMTypeOf(FN_connect_port),
    LLVMTypeOf(FN_activate),
    LLVMTypeOf(FN_run),
    LLVMTypeOf(FN_deactivate),
    LLVMTypeOf(FN_cleanup),
    LLVMTypeOf(FN_extension_data)
  }, 8, 0);

  // Build element pointer for URI String
  LLVMValueRef indexes[] = {
    LLVMConstInt (LLVMInt32Type (), 0, 0),
    LLVMConstInt (LLVMInt32Type (), 0, 0)};

  LLVMValueRef uri_ptr = LLVMConstInBoundsGEP(uri, indexes, 2);

  // Create the field values for the initializer
  LLVMValueRef fields[] = { uri_ptr,
    FN_instantiate,
    FN_connect_port,
    FN_activate,
    FN_run,
    FN_deactivate,
    FN_cleanup,
    FN_extension_data
    };

  LLVMValueRef const_descr = LLVMConstNamedStruct(struct_lv2_descriptor,fields, 8);

  // Tell it to be a global constant
  LLVMSetGlobalConstant(global_descriptor, 1);
  LLVMSetAlignment(global_descriptor, 8);

  // Do the initialization
  LLVMSetInitializer(global_descriptor, const_descr);
}

/*----------------------------------------------------------------------------*/
void emit_code(struct ast *a)
{
  char *error = NULL;

  if (a->nodetype != N_program) {
    fprintf(stderr, "error: top level ast node is not a program\n");
    fprintf(stderr, "nodetype is %d\n", a->nodetype);
    return;
  }

  printf("output filename = '%s'\n", output_filename);

  // Do the setup that's standard
  LLVMModuleRef mod = emit_standard();

  // Do the rest
  emit_options(mod, ((struct prog *) a)->opts);
  emit_declarations(mod, ((struct prog *) a)->decls);
  emit_functions(mod, ((struct prog *) a)->funcs);

  finish_descriptor();

  // It's built, lets verify
  fprintf(stderr, "verifying generated bit code...\n");
  LLVMVerifyModule(mod, LLVMPrintMessageAction, &error);
  LLVMDisposeMessage(error);
  fprintf(stderr, "verify complete\n");

  if (LLVMWriteBitcodeToFile(mod, output_filename) != 0) {
      fprintf(stderr, "error writing bitcode to file\n");
  }

  LLVMDisposeModule(mod);
}
