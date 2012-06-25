#ifndef CODELET_H
#define CODELET_H

#include "ae.h"
#include "strfuncs.h"

typedef struct {
  int numinputs;
  char **inputs;
  char *expression;
} code_type;

typedef struct {
  short result;
  char *name;
  kvlist_type params;
  stringlist_type deps;
  stringlist_type output;
  code_type code;
} codelet_type;

char *makeVariable(lua_State *L, const char *varStr, const kvlist_type pairs);
char *getBlankVariable(const char *varStr);
void addPair(kvlist_type *pairs, const char *key, const char *val);
void addString(stringlist_type *sl, const char *s);
void addCode(code_type *c, const char *def);
short matchesDep(lua_State *L, const codelet_type c, const char *dep, stringlist_type *reqdeps, stringlist_type *outputs);

#endif
