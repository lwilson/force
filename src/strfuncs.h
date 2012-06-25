#ifndef STRFUNCS_H
#define STRFUNCS_H

typedef struct {
  int numparams;
  char **keys;
  char **vals;
} kvlist_type;

typedef struct {
  int size;
  char **values;
} stringlist_type;

char **split(char *str, char *delims, int *numsplits);
char *replace_all(char *origstr, char *pattern, char *replacewith);

#endif
