#include "llvm-c/Core.h"

#include "ast.h"
#include "codegen.h"
#include "sprola.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LLVMContextRef global_context;

LLVMTypeRef struct_plugin;
LLVMTypeRef struct_lv2_descriptor;
LLVMTypeRef struct_lv2_feature;
LLVMValueRef global_descriptor;

LLVMTypeRef void_ptr;
LLVMTypeRef float_ptr;

LLVMValueRef FN_descriptor;
LLVMValueRef FN_instantiate;
LLVMValueRef FN_connect_port;
LLVMValueRef FN_activate;
LLVMValueRef FN_run;
LLVMValueRef FN_deactivate;
LLVMValueRef FN_cleanup;
LLVMValueRef FN_extension_data;

// These functions are for the standard, in every LV2 plugin functions

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
int get_num_ports(struct ast *a)
{
  int result = 0;

  if (a == NULL) {
    return result;
  }

  switch (a->nodetype) {
    case N_program:
      result = get_num_ports(((struct prog *)a)->opts);
      break;
    case N_options:
      result = get_num_ports(a->l) + get_num_ports(a->r);
      break;
    case N_option:
      switch (((struct setopt *)a)->option_flag) {
        case OPT_audio_input:
          result = 1;
          break;
        case OPT_audio_output:
          result = 1;
          break;
        case OPT_control_in:
          result = 1;
          break;
        case OPT_control_out:
          result = 1;
          break;
        default:
          result = 0;
          break;
      }
      break;
    default:
      result = 0;
      break;
  }

  return result;
}

/*----------------------------------------------------------------------------*/
void emit_instantiate(LLVMModuleRef mod, LLVMBuilderRef builder, struct ast *a)
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

  //TODO(jkokosa): add noalias : Define external function malloc
  LLVMTypeRef malloc_type = LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0),
    (LLVMTypeRef []) { LLVMInt64Type()}, 1, 0);
  LLVMValueRef ext_malloc = LLVMAddFunction(mod, "malloc", malloc_type);
  LLVMSetLinkage(ext_malloc, LLVMExternalLinkage);

  int arg_size = get_num_ports(a) * sizeof(float *);

  LLVMValueRef args[] = { LLVMConstInt(LLVMInt64Type(), arg_size, 0) };
  LLVMValueRef result = LLVMBuildCall(builder, ext_malloc, args, 1, "");

  LLVMValueRef cast = LLVMBuildBitCast(builder, result, LLVMPointerType(struct_plugin, 0), "");
  LLVMSetAlignment(LLVMBuildStore(builder, cast, plugin), 8);

  LLVMValueRef y = LLVMBuildLoad(builder, plugin, "");
  LLVMSetAlignment(y, 8);

  LLVMValueRef ret_val = LLVMBuildBitCast(builder, y, LLVMPointerType(LLVMInt8Type(), 0), "");

  LLVMBuildRet(builder, ret_val);
}

/*----------------------------------------------------------------------------*/
void emit_connect_port(LLVMModuleRef mod, LLVMBuilderRef builder, struct ast *a)
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

  //fprintf(stderr, "Port = %s\n", LLVMPrintValueToString(port));

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
  //***********************************************************************
  LLVMBasicBlockRef sw_end = LLVMAppendBasicBlock(FN_connect_port, "");

  // Define switch statement
  LLVMValueRef sw = LLVMBuildSwitch(builder, index, sw_end, 3);

  int num_ports = get_num_ports(a);
  int port_num;
  LLVMValueRef dat;
  LLVMValueRef dat_array;
  LLVMValueRef plug;
  LLVMValueRef tt;
  LLVMBasicBlockRef caseN;

  for (port_num = 0; port_num < num_ports; port_num++) {
    caseN = LLVMInsertBasicBlock(sw_end, "");
    LLVMAddCase(sw, LLVMConstInt(LLVMInt32Type(), port_num, 0), caseN);
    LLVMPositionBuilderAtEnd(builder, caseN);

    // Add body of case here
    dat = LLVMBuildLoad(builder, data, "");
    LLVMSetAlignment(dat, 8);

    dat_array = LLVMBuildBitCast(builder, dat, LLVMPointerType(LLVMFloatType(), 0), "");

    plug = LLVMBuildLoad(builder, plugin, "");
    LLVMSetAlignment(plug, 8);

    tt = LLVMBuildStructGEP2(builder, struct_plugin, plug, port_num, "");
    LLVMSetAlignment(LLVMBuildStore(builder, dat_array, tt), 8);

    LLVMBuildBr(builder, sw_end);
  }

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
// loop through options to get all input and output ports
//  and allocate space, store into them, and update
//  symbol table to their locations
/*----------------------------------------------------------------------------*/
void emit_run_ports(LLVMBuilderRef builder, LLVMValueRef mem_plugin)
{
  LLVMValueRef mem_port = NULL;
  LLVMValueRef tt = NULL;
  LLVMValueRef xx = NULL;
  LLVMValueRef yy = NULL;

  struct symbol *sym;

  if (info->num_ports == 0) {
    fprintf(stderr, "Error - expected ports in info structure\n");
  }

  LLVMValueRef plug = LLVMBuildLoad(builder, mem_plugin, "");
  LLVMSetAlignment(plug, 8);

  for (int i = 0; i < info->num_ports; ++i) {
    sym = lookup(info->port[i].symbol);

    if (strcmp(info->port[i].data_type, PORT_TYPE_AUDIO) == 0) {
      mem_port = LLVMBuildAlloca(builder, LLVMPointerType(LLVMFloatType(), 0), sym->name);
      LLVMSetAlignment(mem_port, 8);
      tt = LLVMBuildStructGEP2(builder, struct_plugin, plug, i, "");
      yy = LLVMBuildLoad(builder, tt, "");
    }

    if (strcmp(info->port[i].data_type, PORT_TYPE_CONTROL) == 0) {
      mem_port = LLVMBuildAlloca(builder, LLVMFloatType(), sym->name);
      LLVMSetAlignment(mem_port, 4);
      tt = LLVMBuildStructGEP2(builder, struct_plugin, plug, i, "");
      xx = LLVMBuildLoad(builder, tt, "");
      yy = LLVMBuildLoad(builder, xx, "");
    }

    LLVMBuildStore(builder, yy, mem_port);
    sym->value = mem_port;
  }
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

  LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(FN_run, "entry");
  LLVMBasicBlockRef user_block = LLVMAppendBasicBlock(FN_run, "user");
  LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(FN_run, "exit");

  // work on entry block
  LLVMPositionBuilderAtEnd(builder, entry_block);

  // Allocate space on stack for function parameters
  LLVMValueRef mem_instance = LLVMBuildAlloca(builder, LLVMPointerType(LLVMInt8Type(), 0), "-instance");
  LLVMValueRef mem_n_samples = LLVMBuildAlloca(builder, LLVMInt32Type(), "-n_samples");

  // store parameter pointers
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_run, 0), mem_instance), 8);
  LLVMSetAlignment(LLVMBuildStore(builder, LLVMGetParam(FN_run, 1), mem_n_samples), 4);

  // connect parameter to Sprola pre-defined variable Num_Samples
  struct symbol *sym = lookup("Num_Samples");
  sym->value = mem_n_samples;

  // Allocate space on stack for plugin instance structure
  LLVMValueRef mem_plugin = LLVMBuildAlloca(builder, LLVMPointerType(struct_plugin, 0), "plugin");
  // store the plugin values into the allocated space
  LLVMValueRef plug_ptr = LLVMBuildLoad(builder, mem_instance, "");
  LLVMValueRef cast = LLVMBuildBitCast(builder, plug_ptr, LLVMPointerType(struct_plugin, 0), "");
  LLVMSetAlignment(LLVMBuildStore(builder, cast, mem_plugin), 8);

  // Allocate space on stack for plugin ports
  emit_run_ports(builder, mem_plugin);

  LLVMSetAlignment(mem_instance, 8);
  LLVMSetAlignment(mem_n_samples, 4);
  LLVMSetAlignment(mem_plugin, 8);
  LLVMSetAlignment(plug_ptr, 8);

  // Branch to user block
  LLVMBuildBr(builder, user_block);

  // work on user block, user code will be added here later
  LLVMPositionBuilderAtEnd(builder, user_block);
  // when finished with user code go to exit block
  LLVMBuildBr(builder, exit_block);

  // work on exit block
  LLVMPositionBuilderAtEnd(builder, exit_block);
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
  LLVMBuildCall(builder, ext_free, args, 1, "");

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
void finish_descriptor(LLVMValueRef uri)
{
  if (verbose_flag) {
    fprintf(stderr, "Finishing descriptor...\n");
    fflush(trace_file);
  }

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
LLVMModuleRef emit_standard(struct ast *a)
{
  if (verbose_flag) {
    fprintf(stderr, "emitting standard functions...\n");
  }

  void_ptr = LLVMPointerType(LLVMInt8Type(), 0);
  float_ptr = LLVMPointerType(LLVMFloatType(), 0);

  global_context = LLVMGetGlobalContext();
  LLVMModuleRef mod = LLVMModuleCreateWithNameInContext(current_filename, global_context);

  // -------
  // This is defined by the LV2 specification
  struct_lv2_feature = LLVMStructCreateNamed(global_context, "struct._LV2_Feature");

  LLVMTypeRef lv2_feature_body[] = { void_ptr, void_ptr };
  LLVMStructSetBody(struct_lv2_feature, lv2_feature_body, 2, 0);

  // -------
  // This is defined by the LV2 specification, we'll fill body
  // after the functions created
  // -------
  struct_lv2_descriptor = LLVMStructCreateNamed(global_context, "struct._LV2_Descriptor");

  // -------
  // This is the structure that holds all the global fields, we'll fill
  // the body with the defined ports and all the global variables
  //TODO(jkokosa) scan for global variables
  // -------
  struct_plugin = LLVMStructCreateNamed(global_context, "struct.Plugin");

  int num_ports = get_num_ports(a);

  if (num_ports < 1) {
    fprintf(stderr, "Error - no ports found in input source\n");
    exit(1);
  }

  LLVMTypeRef plugin_body[num_ports];

  for (int i = 0; i < num_ports; ++i) {
    plugin_body[i] = LLVMPointerType(LLVMFloatType(), 0);
  }
  LLVMStructSetBody(struct_plugin, plugin_body, num_ports, 0);

  LLVMBuilderRef builder = LLVMCreateBuilderInContext(global_context);

  fflush(trace_file);
  emit_lv2_descriptor(mod, builder);
  fflush(trace_file);
  emit_instantiate(mod, builder, a);
  fflush(trace_file);
  emit_connect_port(mod, builder, a);
  fflush(trace_file);
  emit_activate(mod, builder);
  fflush(trace_file);
  emit_run(mod, builder);
  fflush(trace_file);
  emit_deactivate(mod, builder);
  fflush(trace_file);
  emit_cleanup(mod, builder);
  fflush(trace_file);
  emit_extension_data(mod, builder);
  fflush(trace_file);

  LLVMDisposeBuilder(builder);

  return(mod);
}
