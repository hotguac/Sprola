#include "llvm-c/Core.h"

#include "codegen.h"
#include "sprola.h"

#include <assert.h>
#include <bsd/string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char uri_full[MAX_FILENAME_SIZE];

/*----------------------------------------------------------------------------*/
char* get_uri(struct ast *a, struct plugin_filenames *names)
{
  if (verbose_flag) {
    fprintf(stderr, "get_uri...\n");
  }

  struct setopt *r = NULL;
  memset(uri_full, 0, MAX_FILENAME_SIZE);

  int search_done = 0;

  while (!search_done) {
      if (a == NULL) {
        fprintf(stderr, "Error - Empty AST\n");
        exit(1);
      }

      switch (a->nodetype) {
        case N_program:
          if (((struct prog *)a)->opts == NULL) {
            search_done = 1;
            fprintf(stderr, "Warning - no %%option URI <uri>' found, using default value ");
            fprintf(stderr, "of http:/example.org/plugins\n");
            strlcpy(uri_full, "<http://example.org/plugins", MAX_FILENAME_SIZE);
          } else {
            a = ((struct prog *)a)->opts;
          }
          break;
        case N_options:
          r = (struct setopt *)a->r;
          if (r->option_flag == OPT_uri) {
            a = a->r;
          } else {
            a = a->l;
          }
          break;
        case N_option:
          if (((struct setopt *)a)->option_flag == OPT_uri) {
            search_done = 1;
            strlcpy(uri_full, "<", MAX_FILENAME_SIZE);
            char temp[MAX_FILENAME_SIZE];
            memset(temp, 0, MAX_FILENAME_SIZE);

            assert (r != NULL);
            struct symref *ref = (struct symref *)(r->target);
            assert (ref != NULL);
            strlcpy(temp, ref->sym->name, MAX_FILENAME_SIZE);

            if (strlen(temp) > 0) {
                temp[strlen(temp) - 1] = 0;
            }

            strlcat(uri_full, temp+1, MAX_FILENAME_SIZE);
          }
          break;
        default:
          fprintf(stderr, "Error - Unexpected node type %d\n", a->nodetype);
          exit(1);
          break;
      }

  }

  //TODO(jkokosa) append plugin name to supplied URI and use in generated IR
  //strlcat(uri_full, "/", MAX_FILENAME_SIZE);
  //strlcat(uri_full, names->plugin_name, MAX_FILENAME_SIZE);
  strlcat(uri_full, ">", MAX_FILENAME_SIZE);

  return(uri_full);
}

/*----------------------------------------------------------------------------*/
void emit_plugin_ttl(LLVMModuleRef mod, struct ast *a, struct plugin_filenames *names)
{
  if (verbose_flag) {
    fprintf(stderr, "emit_plugin_ttl...\n");
  }

  if (a == NULL) {
    fprintf(stderr, "Error - Empty AST\n");
    return;
  }

  if (names->plugin_ttlname[0] == 0) {
    fprintf(stderr, "Error - plugin ttl filename is empty\n");
    return;
  }

  FILE *fd = fopen(names->plugin_ttlname, "w");

  if (fd == NULL) {
    fprintf(stderr, "Error - error opening plugin ttl '%s'\n", names->plugin_ttlname);
    exit(1);
  }

  char *uri = get_uri(a, names);

  fprintf(fd, "# The full description of the plugin is in this file.\n\n");

  fprintf(fd, "# ==== Namespace Prefixes ====\n\n");
  fprintf(fd, "@prefix doap:  <http://usefulinc.com/ns/doap#> .\n");
  fprintf(fd, "@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n");
  fprintf(fd, "@prefix rdf:   <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n\n");
  fprintf(fd, "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n\n");
  fprintf(fd, "@prefix units: <http://lv2plug.in/ns/extensions/units#> .\n\n");

  fprintf(fd, "# First the type of the plugin is described.\n\n");
  fprintf(fd, "%s\n\ta lv2:Plugin ;\n\n", uri);

  fprintf(fd, "# Plugins are associated with a project.\n\n");
  fprintf(fd, "#\tlv2:project %s\n\n", names->plugin_name);

  fprintf(fd, "# Every plugin must have a name, described with the doap:name property.\n\n");
  fprintf(fd, "\tdoap:name \"Sprola %s\";\n\n", names->plugin_name);

  fprintf(fd, "# Every port must have at least two types, one that specifies direction and one for data type.\n\n");
  fprintf(fd, "\tlv2:port [\n");

  //struct port_info *info = get_port_info(a);

  //TODO(jkokosa) handle trailing ';' and ']' correctly in ttl files
  for (int num = 0; num < info->num_ports; ++num) {
    fprintf(fd, "\t\ta lv2:%s ,\n", info->port[num].direction);
    fprintf(fd, "\t\t  lv2:%s ;\n", info->port[num].data_type);
    fprintf(fd, "\t\tlv2:index %d ;\n", num);
    fprintf(fd, "\t\tlv2:symbol \"%s\" ;\n", info->port[num].symbol);
    fprintf(fd, "\t\tlv2:name \"%s\" ;\n", info->port[num].symbol);

    if ((strcmp(info->port[num].data_type, PORT_TYPE_CONTROL) == 0) &&
        (strcmp(info->port[num].direction, PORT_DIRECTION_IN) == 0)) {
          //TODO(jkokosa) specify these values on %option lines
          fprintf(fd, "\t\t\tlv2:default 1.5 ;\n");
          fprintf(fd, "\t\t\tlv2:minimum 0.0 ;\n");
          fprintf(fd, "\t\t\tlv2:maximum +10.0 ;\n");
        }

    if ((num+1) < info->num_ports) {
      fprintf(fd, "\t\t] , [\n");
    } else {
      fprintf(fd, "\t\t] .\n");
    }

  }

  fclose(fd);
}

/*----------------------------------------------------------------------------*/
void emit_manifest_ttl(LLVMModuleRef mod, struct ast *a, struct plugin_filenames *names)
{
  if (verbose_flag) {
    fprintf(stderr, "emit_manifest_ttl...\n");
  }


  if (a == NULL) {
    fprintf(stderr, "Error - Empty AST\n");
    exit(1);
  }

  if (names->manifest_ttlname[0] == 0) {
    fprintf(stderr, "Error - manifest ttl filename is empty\n");
    exit(1);
  }

  FILE *fd = fopen(names->manifest_ttlname, "w");

  if (fd == NULL) {
    fprintf(stderr, "Error - error opening manifest ttl '%s'\n", names->manifest_ttlname);
    exit(1);
  }

  char *uri = get_uri(a, names);

  fprintf(fd, "# ==== Namespace Prefixes ====\n\n");
  fprintf(fd, "@prefix lv2:  <http://lv2plug.in/ns/lv2core#> .\n");
  fprintf(fd, "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n\n");
  fprintf(fd, "# ==== A Plugin Entry ====\n\n");
  fprintf(fd, "%s\n\ta lv2:Plugin ;\n", uri);
  fprintf(fd, "\tlv2:binary <%s.so> ;\n", names->plugin_name);
  fprintf(fd, "\trdfs:seeAlso <%s.ttl> .\n\n", names->plugin_name);

  fclose(fd);
}
