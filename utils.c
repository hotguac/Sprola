#include "llvm-c/BitWriter.h"
#include "llvm-c/Core.h"
#include "llvm-c/Target.h"

#include <bsd/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "codegen.h"
#include "sprola.h"
#include "utils.h"

struct plugin_filenames names;

/*
  name : a preallocated array of chars big enough to take output
    which will be the path stripped of any extension and with an
    added extension of '.bc'
  path : the full path to the input filename
*/

/*----------------------------------------------------------------------------*/
int build_names(char *path, struct plugin_filenames *names)
{
  memset(names, 0, sizeof(struct plugin_filenames));

  get_plugin_name(names->plugin_name, path);

  strlcpy(names->bc_filename, names->plugin_name, MAX_FILENAME_SIZE);
  strlcpy(names->ll_filename, names->plugin_name, MAX_FILENAME_SIZE);
  strlcpy(names->trace_filename, names->plugin_name, MAX_FILENAME_SIZE);
  strlcpy(names->bundle_dirname, names->plugin_name, MAX_FILENAME_SIZE);

  strlcat(names->trace_filename, ".trace", MAX_FILENAME_SIZE);
  size_t x = strlcat(names->bc_filename, ".bc", MAX_FILENAME_SIZE);
  int y = strlen(names->bc_filename);

  if (x > y) {
    printf("WTF! %zu %d\n",x, y);
  }

  strlcat(names->ll_filename, ".ll", MAX_FILENAME_SIZE);
  strlcat(names->bundle_dirname, ".lv2/", MAX_FILENAME_SIZE);

  strlcpy(names->plugin_ttlname, names->bundle_dirname, MAX_FILENAME_SIZE);
  strlcpy(names->so_filename, names->bundle_dirname, MAX_FILENAME_SIZE);
  strlcpy(names->manifest_ttlname, names->bundle_dirname, MAX_FILENAME_SIZE);

  strlcat(names->plugin_ttlname, names->plugin_name, MAX_FILENAME_SIZE);
  strlcat(names->plugin_ttlname, ".ttl", MAX_FILENAME_SIZE);

  strlcat(names->manifest_ttlname, "manifest.ttl", MAX_FILENAME_SIZE);

  strlcat(names->so_filename, names->plugin_name, MAX_FILENAME_SIZE);
  strlcat(names->so_filename, ".so", MAX_FILENAME_SIZE);

  if (verbose_flag) {
    fprintf(stderr, "bc_filename = '%s'\n", names->bc_filename);
    fprintf(stderr, "ll_filename = '%s'\n", names->ll_filename);
    fprintf(stderr, "bundle_dirname = '%s'\n", names->bundle_dirname);
    fprintf(stderr, "plugin_ttlname = '%s'\n", names->plugin_ttlname);
    fprintf(stderr, "manifest_ttlname = ' %s'\n", names->manifest_ttlname);
    fprintf(stderr, "so_filename = '%s'\n", names->so_filename);
    fprintf(stderr, "current_filename = '%s'\n", path);
    fprintf(stderr, "plugin name = '%s'\n", names->plugin_name);
  }

  return 0;
}

/*----------------------------------------------------------------------------*/
int get_plugin_name(char *name, char *path)
{
  int i;
  int length;
  int dot_position = -1;
  int slash_position = -1;

  name[0] = 0;
  length = strlen(path);

  if (length == 0) {
    return 0;
  }

  for (i = length -1; i >= 0; --i) {
    switch (path[i]) {
      case '.':
        // found start of extension
        if ((dot_position < 0) && (slash_position < 0)) {
          dot_position = i;
        }
        break;
      case '/':
        // found front of name
        if (slash_position < 0) {
          slash_position = i;
        }
        break;
      default:
        break;
    }
  }

  if (dot_position > slash_position) {
    strncpy(name, path+(slash_position+1), dot_position-(slash_position+1));
    name[dot_position-(slash_position+1)] = 0;
  } else {
    if (slash_position >= 0) {
      strncpy(name, path+(slash_position+1), length - (slash_position+1));
      name[length-(slash_position+1)] = 0;
    }
  }

  if (verbose_flag) {
    printf("path = '%s'\n", path);
    printf("dot = %d\n", dot_position);
    printf("slash = %d\n", slash_position);
    printf("plugin name = '%s'\n", name);
  }

  return strlen(name);
}

//----------------------------------------------------------------------------
//TODO(jkokosa) - run optimize passes including 'memory to register promotion'
//----------------------------------------------------------------------------
void generate_obj_lib(LLVMModuleRef mod, struct plugin_filenames *names)
{
  char command[MAX_FILENAME_SIZE+100];

  struct stat st = {0};
  int mkdir_result;

  if (stat(names->bundle_dirname, &st) == -1) {
    if (verbose_flag) {
      printf("creating bundle directory '%s'\n", names->bundle_dirname);
    }
    mkdir_result = mkdir(names->bundle_dirname, S_IRWXU);
    if (mkdir_result != 0) {
      fprintf(stderr, "error creating bundle directory '%s' status=%d\n", names->bundle_dirname, mkdir_result);
    }
  } else {
    if (verbose_flag) {
      printf("bundle directory '%s' already exists\n", names->bundle_dirname);
    }
  }

  if (LLVMWriteBitcodeToFile(mod, names->bc_filename) != 0) {
    fprintf(stderr, "error writing bitcode to file %s\n", names->bc_filename);
  }

  strlcpy(command, "clang -fPIC -shared ", MAX_FILENAME_SIZE+100);
  strlcat(command, names->bc_filename, MAX_FILENAME_SIZE+100);
  strlcat(command, " -o ", MAX_FILENAME_SIZE+100);
  strlcat(command, names->so_filename, MAX_FILENAME_SIZE+100);

  if (verbose_flag) {
    printf("clang command is '%s'\n", command);
  }

  FILE* file = popen(command, "r"); // NOLINT
  if (file == NULL) {
    printf("error generating .so file '%s'", names->so_filename);
  }

  pclose(file);

  if (ll_flag) {
    strlcpy(command, "llvm-dis ", MAX_FILENAME_SIZE+100);
    strlcat(command, names->bc_filename, MAX_FILENAME_SIZE+100);
    strlcat(command, " -o=", MAX_FILENAME_SIZE+100);
    strlcat(command, names->ll_filename, MAX_FILENAME_SIZE+100);

    if (verbose_flag) {
      printf("llvm command is '%s'\n", command);
    }

    FILE* file = popen(command, "r"); // NOLINT
    if (file == NULL) {
      printf("error generating .ll file '%s'", names->ll_filename);
    }

    pclose(file);
  }

}

/*----------------------------------------------------------------------------*/
struct port_info *info;

void add_port_info(struct ast *a) {
  int found;
  struct setopt *option;

  if (a == NULL) {
    return;
  }

  switch (a->nodetype) {
    case N_program:
      add_port_info(((struct prog *)a)->opts);
      break;
    case N_options:
      add_port_info(a->l);
      add_port_info(a->r);
      break;
    case N_option:
      option = (struct setopt *) a;
      switch (option->option_flag) {
        case OPT_audio_input:
          strlcpy(info->port[info->num_ports].direction, PORT_DIRECTION_IN, MAX_PORT_ATTR_SIZE);
          strlcpy(info->port[info->num_ports].data_type, PORT_TYPE_AUDIO, MAX_PORT_ATTR_SIZE);
          found = 1;
          break;
        case OPT_audio_output:
          strlcpy(info->port[info->num_ports].direction, PORT_DIRECTION_OUT, MAX_PORT_ATTR_SIZE);
          strlcpy(info->port[info->num_ports].data_type, PORT_TYPE_AUDIO, MAX_PORT_ATTR_SIZE);
          found = 1;
          break;
        case OPT_control_in:
          strlcpy(info->port[info->num_ports].direction, PORT_DIRECTION_IN, MAX_PORT_ATTR_SIZE);
          strlcpy(info->port[info->num_ports].data_type, PORT_TYPE_CONTROL, MAX_PORT_ATTR_SIZE);
          found = 1;
          break;
        case OPT_control_out:
          strlcpy(info->port[info->num_ports].direction, PORT_DIRECTION_OUT, MAX_PORT_ATTR_SIZE);
          strlcpy(info->port[info->num_ports].data_type, PORT_TYPE_CONTROL, MAX_PORT_ATTR_SIZE);
          found = 1;
          break;
        default:
          found = 0;
          break;
      }

      if (found) {

        //TODO(jkokosa) refactor out a function
        if (option->value1 == NULL) {
          info->port[info->num_ports].def = 0.5;
        } else {
          switch (option->value1->nodetype) {
            case N_float:
              if (trace_flag) {
                fprintf(trace_file, "found float for option value1\n");
              }

              info->port[info->num_ports].def = ((struct floatval *) option->value1)->number;
              break;
            default:
              fprintf(stderr, "expected float found %d\n", option->value1->nodetype);
          }
        }

        if (option->value2 == NULL) {
          info->port[info->num_ports].min = 0.0;
        } else {
          switch (option->value2->nodetype) {
            case N_float:
              if (trace_flag) {
                fprintf(trace_file, "found float for option value2\n");
              }

              info->port[info->num_ports].min = ((struct floatval *) option->value2)->number;
              break;
            default:
              fprintf(stderr, "expected float found %d\n", option->value2->nodetype);
          }
        }

        if (option->value3 == NULL) {
          info->port[info->num_ports].max = 1.0;
        } else {
          switch (option->value3->nodetype) {
            case N_float:
              if (trace_flag) {
                fprintf(trace_file, "found float for option value3\n");
              }

              info->port[info->num_ports].max = ((struct floatval *) option->value3)->number;
              break;
            default:
              fprintf(stderr, "expected float found %d\n", option->value3->nodetype);
          }
        }

        struct ast *target = option->target;
        if (target->nodetype != N_symbol_ref) {
          fprintf(stderr, "Error - expected N_symbol_ref found %d\n", target->nodetype);
          exit(1);
        }

        struct symbol *sym = ((struct symref *)target)->sym;
        strlcpy(info->port[info->num_ports].symbol, sym->name, MAX_PORT_ATTR_SIZE);
        info->num_ports++;
      }

      break;
    default:
      break;
  }

}

struct port_info *get_port_info(struct ast *a)
{
  if (a == NULL) {
    fprintf(stderr, "Error - empty ast tree\n");
    exit(1);
  }

  info = (struct port_info *) malloc(sizeof(struct port_info));

  info->num_ports = 0;

  add_port_info(a);

  if (info->num_ports == 0) {
    fprintf(stderr, "Error - no options found\n");
    exit(1);
  }

  return info;
}

void dump_current_function(LLVMBuilderRef builder) {
  fprintf(stderr, "------------------------------------\n");
  fprintf(stderr, "%s\n", LLVMPrintValueToString(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder))));
  fprintf(stderr, "------------------------------------\n");
}

void trace(char *message) {
  if (trace_flag) {
    fprintf(trace_file, "%s", message);
    fflush(trace_file);
  }
}
