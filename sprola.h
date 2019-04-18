/* Signal PROcessing LAnguage */
enum symbol_type {FUNCTION_NAME, VARIABLE_NAME, MEMBER_NAME};

struct symbol {
  char *name;
  struct ref *reflist;
  enum symbol_type sym_type;
};

struct ref {
  struct ref *next;
  char *filename;
  int flags;
  int lineno;
};

#define NHASH 9997

struct symbol *lookup(char* name);
void addref(int lineno, char* filename, char* name, int flags);
