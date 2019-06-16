#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

extern int verbose_flag;

/*
  name : a preallocated array of chars big enough to take output
    which will be the path stripped of any extension and with an
    added extension of '.bc'
  path : the full path to the input filename
*/

int build_names(char *path, struct plugin_filenames *names)
{
  memset(names, 0, sizeof(struct plugin_filenames));

  get_plugin_name(names->plugin_name, path);
  printf("in build names names->plugin_Name = '%s'\n", names->plugin_name);

  strcpy(names->output_filename, names->plugin_name);
  strcat(names->output_filename, ".bc");

  strcpy(names->bc_filename, names->plugin_name);
  strcpy(names->ll_filename, names->plugin_name);
  strcpy(names->bundle_dirname, names->plugin_name);

  strcat(names->bc_filename, ".bc");
  strcat(names->ll_filename, ".ll");
  strcat(names->bundle_dirname, ".lv2/");

  strcpy(names->plugin_ttlname, names->bundle_dirname);
  strcpy(names->so_filename, names->bundle_dirname);
  strcpy(names->manifest_ttlname, names->bundle_dirname);

  strcat(names->plugin_ttlname, names->plugin_name);
  strcat(names->plugin_ttlname, ".ttl");

  strcat(names->manifest_ttlname, "manifest.ttl");

  strcat(names->so_filename, names->plugin_name);
  strcat(names->so_filename, ".so");

  if (verbose_flag) {
    printf("bc_filename = '%s'\n", names->bc_filename);
    printf("ll_filename = '%s'\n", names->ll_filename);
    printf("bundle_dirname = '%s'\n", names->bundle_dirname);
    printf("plugin_ttlname = '%s'\n", names->plugin_ttlname);
    printf("manifest_ttlname = ' %s'\n", names->manifest_ttlname);
    printf("so_filename = '%s'\n", names->so_filename);
    printf("current_filename = '%s'\n", path);
    printf("plugin name = '%s'\n", names->plugin_name);
    printf("output filename = '%s'\n", names->output_filename);
  }

  return 0;
}

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
