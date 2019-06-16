#ifndef __util__
#define __util__

#define MAX_FILENAME_SIZE 120

struct plugin_filenames  {
  char output_filename[MAX_FILENAME_SIZE];    // write .ll output here
  char plugin_name[MAX_FILENAME_SIZE];    // write .ll output here
  char bc_filename[MAX_FILENAME_SIZE];
  char ll_filename[MAX_FILENAME_SIZE];
  char bundle_dirname[MAX_FILENAME_SIZE];
  char manifest_ttlname[MAX_FILENAME_SIZE];
  char plugin_ttlname[MAX_FILENAME_SIZE];
  char so_filename[MAX_FILENAME_SIZE];
};

int get_plugin_name(char *name, char *path);
int build_names(char *path, struct plugin_filenames *names);

#endif
