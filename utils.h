#ifndef UTILS_H
#define UTILS_H

#include "llvm-c/Core.h"

#include "ast.h"

#define MAX_FILENAME_SIZE 120
#define MAX_NUMBER_PORTS 64
#define MAX_PORT_ATTR_SIZE 24

struct plugin_filenames  {
  char plugin_name[MAX_FILENAME_SIZE];
  char bc_filename[MAX_FILENAME_SIZE];
  char ll_filename[MAX_FILENAME_SIZE];
  char bundle_dirname[MAX_FILENAME_SIZE];
  char manifest_ttlname[MAX_FILENAME_SIZE];
  char plugin_ttlname[MAX_FILENAME_SIZE];
  char so_filename[MAX_FILENAME_SIZE];
};

struct a_port {
  char direction[MAX_PORT_ATTR_SIZE];
  char data_type[MAX_PORT_ATTR_SIZE];
  char symbol[MAX_PORT_ATTR_SIZE];
};

struct port_info {
  int num_ports;
  struct a_port port[MAX_NUMBER_PORTS];
};

extern struct plugin_filenames names;

int get_plugin_name(char *name, char *path);
int build_names(char *path, struct plugin_filenames *names);
void generate_obj_lib(LLVMModuleRef mod, struct plugin_filenames *names);
struct port_info *get_port_info(struct ast *a);

void dump_current_function(LLVMBuilderRef builder);


#endif
