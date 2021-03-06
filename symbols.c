#include "codegen.h"
#include "sprola.h"
#include "symbols.h"

#include <bsd/string.h>
#include <stdio.h>
#include <stdlib.h>

struct symbol symbol_table[NHASH];

/*----------------------------------------------------------------------------*/
/* hash a symbol */
/*----------------------------------------------------------------------------*/
static unsigned symhash(char *id)
{
  unsigned int hash = 0;
  unsigned c;

  while ((c = *id++)) {
    hash = hash*9 ^ c;
  }

  return hash;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
struct symbol *lookup(char* id)
{
  struct symbol *sp = &symbol_table[symhash(id)%NHASH];
  int scount = NHASH;

  while (--scount >= 0) {
    if (sp->name && !strcmp(sp->name, id)) { // Got a match
      return sp;
    }

    if (!sp->name) { // Empty slot where it should be so add it
      sp->name = strdup(id);
      sp->func = NULL;
      sp->syms = NULL;
      sp->reflist = NULL;
      sp->value = NULL;
      return sp;
    }

    if (++sp >= symbol_table+NHASH) { // Wrap back to beginning to of table
      sp = symbol_table;
    }
  }

  fputs("Symbol table overflow\n", stderr);
  abort();
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void addref(int lineno, char *filename, char *id, int flags)
{
  if (trace_flag) {
    fprintf(stderr, "\t\t addref %d, %s, %s, %d\n", lineno, filename, id, flags);
  }

  struct ref *r;
  struct symbol *sp = lookup(id);

  /* don't do dups */
  if(sp->reflist &&
     sp->reflist->lineno == lineno &&
     sp->reflist->filename == filename) {
     return;
  }

  r = (struct ref *) malloc(sizeof(struct ref));

  if(!r) {
    fputs("out of space\n", stderr); abort();
  }

  r->next = sp->reflist;
  r->filename = filename;
  r->lineno = lineno;
  r->flags = flags;

  sp->reflist = r;
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void printrefs()
{
  printf("Symbol References:\n");

  struct symbol *sp;

  /* sort the symbol table */
  // qsort(symbol_table, NHASH, sizeof(struct symbol), symcompare);

  for (sp = symbol_table; sp < symbol_table+NHASH; sp++) {
    if (sp->name) {
      char *prevfn = NULL;	/* last printed filename, to skip dups */

      /* reverse the list of references */
      struct ref *rp = sp->reflist;
      struct ref *rpp = 0;	/* previous ref */
      struct ref *rpn;	/* next ref */

      do {
        rpn = rp->next;
        rp->next = rpp;
        rpp = rp;
        rp = rpn;
      } while(rp);

      /* now print the word and its references */
      printf("%10s", sp->name);

      for (rp = rpp; rp; rp = rp->next) {
        if (rp->filename == prevfn) {
  	     printf(" %d", rp->lineno);
        } else {
          printf(" %s:%d", rp->filename, rp->lineno);
  	      prevfn = rp->filename;
        }
      } /* end for */

      printf("\n");
    }
  }
}
