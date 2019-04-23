/* Signal PROcessing LAnguage */
#ifndef __SPROLA__
#define __SPROLA__


struct symbol *lookup(char* name);
void addref(int lineno, char* filename, char* name, int flags);

#endif
