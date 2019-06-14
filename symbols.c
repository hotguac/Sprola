#include "sprola.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "symbols.h"

struct symbol symbol_table[NHASH];

/*----------------------------------------------------------------------------*/
/* hash a symbol */
/*----------------------------------------------------------------------------*/
static unsigned symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while ((c = *sym++)) {
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

    if (!sp->name) { //  This slot is available
      sp->name = strdup(id);
      sp->reflist = 0;
      return sp;
    }

    if (++sp >= symbol_table+NHASH) { // Wrap back to beginning to find empty slot
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
  //printf("\t\t addref %d, %s, %s, %d\n", lineno, filename, id, flags);

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
/* aux function for sorting */
/*----------------------------------------------------------------------------*/
static int symcompare(const void *xa, const void *xb)
{
  const struct symbol *a = (struct symbol *) xa;
  const struct symbol *b = (struct symbol *) xb;

  if (!a->name) {
    if (!b->name) {
      return 0;	/* both empty */
    }
    return 1;			/* put empties at the end */
  }

  if (!b->name) {
    return -1;
  }

  return strcmp(a->name, b->name);
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
