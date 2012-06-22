#include <stdlib.h>
#include <string.h>

char **split(char *str, char *delims, int *numsplits) {
  char nsplits = 0;
  char *result = NULL;
  char **tmpsplit = NULL;

  result = strtok(str, delims);
  while(result != NULL) {
    tmpsplit = realloc(tmpsplit, sizeof(char**)*(nsplits+1));
    tmpsplit[nsplits] = (char*)malloc(sizeof(char*)*(strlen(result)+1));
    strcpy(tmpsplit[nsplits], result);
    nsplits++;
    result = strtok(NULL, delims);
  }

  *numsplits = nsplits;
  return tmpsplit;
}

char *replace_all(char *origstr, char *pattern, char *replacewith) {
  char *newstr = NULL;
  char *cptr = origstr;

  char* findstr = strstr(cptr, pattern);
  while(findstr != NULL) {
    //Allocate additional space for all of the contents between cptr and findstr
    if(newstr == NULL) newstr = malloc(sizeof(char)*(strlen(replacewith)+1));
    else newstr = realloc(newstr, sizeof(char)*(strlen(newstr)+strlen(replacewith)+1));
    //Copy all of the contents of the string between cptr and findstr
    strncat(newstr, cptr, findstr-cptr);
    //Copy replace string into newstr
    strcat(newstr, replacewith);
    //Move cptr to location past pattern
    cptr = findstr + strlen(pattern);
    //Find the next occurrence of pattern
    findstr = strstr(cptr, pattern);
  }

  //Clean up the tail of the string
  newstr = realloc(newstr, sizeof(char)*(strlen(newstr)+strlen(cptr)+1));
  strcat(newstr, cptr);

  return newstr;
}
