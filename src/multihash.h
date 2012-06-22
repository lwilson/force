#ifndef _MULTIHASH_H
#define _MULTIHASH_H
#include <omp.h>
#include <stdlib.h>

typedef struct entrystruct entrytype;

struct entrystruct{
  char *key;
  char *value;
  entrytype *next;
};

typedef struct {
  entrytype *front;
  entrytype *back;
} buckettype;

typedef struct {
  omp_lock_t lock;
  buckettype buckets[8192];
} hashtype;

typedef struct {
  unsigned long size;
  hashtype *multihash;
} multihashtype;

multihashtype multihash_create(const int size) {
  multihashtype retval;
  int i,j;
  retval.size = size;
  retval.multihash = (hashtype *)malloc(sizeof(hashtype) * size);
  for(i=0; i<size; i++) {
    //Initialize the OMP lock
    omp_init_lock(&retval.multihash[i].lock); 
    //Initialize the collision pointers
    for(j=0; j<8192; j++) 
      retval.multihash[i].buckets[j].front = retval.multihash[i].buckets[j].back = NULL;
  }
  return retval;
}

void multihash_destroy(multihashtype hashtable) {
  int i,j;
  entrytype *tmp;
  for(i=0; i<hashtable.size; i++) {
    //Free all of the memory
    omp_destroy_lock(&hashtable.multihash[i].lock);
    for(j=0; j<8192; j++) {
      while(hashtable.multihash[i].buckets[j].front != NULL) {
        tmp = hashtable.multihash[i].buckets[j].front;
        hashtable.multihash[i].buckets[j].front = hashtable.multihash[i].buckets[j].front->next;
        free(tmp);
      }
    }
  }
}

unsigned long hash(const char *key) {
  int len = strlen(key);
  int i;
  unsigned long hashval = 0;
  for(i=0; i<len; i++) {
    hashval += (unsigned long)key[i];
  }

  return hashval;
}

char *get(const multihashtype hashtable, const char *key) {
  unsigned long hval = hash(key);
  char *retval;

  entrytype *tmp = hashtable.multihash[hval % hashtable.size].buckets[hval % 8192].front;
  while(tmp != NULL) {
    if(strcmp(key, tmp->key) == 0) {
      retval = (char*)malloc(sizeof(char)*(strlen(tmp->value)+1));
      strcpy(retval, tmp->value);
      return retval;
    }
    tmp = tmp->next;
  }

  return NULL;
}

void put(const multihashtype hashtable, const char *key, const char *value) {
  unsigned long hval = hash(key);

  //Lock the bucket
  omp_set_lock(&hashtable.multihash[hval % hashtable.size].lock);

  //Insert the value in the approprate bucket
  entrytype *tmp = (entrytype*)malloc(sizeof(entrytype));
  tmp->next = NULL;
  tmp->key = (char*)malloc(sizeof(char)*(strlen(key)+1));
  strcpy(tmp->key, key);
  tmp->value = (char*)malloc(sizeof(char)*(strlen(value)+1));
  strcpy(tmp->value, value);
  if(hashtable.multihash[hval % hashtable.size].buckets[hval % 8192].back == NULL)
    hashtable.multihash[hval % hashtable.size].buckets[hval % 8192].front = tmp;
  else 
    hashtable.multihash[hval % hashtable.size].buckets[hval % 8192].back->next = tmp;
  hashtable.multihash[hval % hashtable.size].buckets[hval % 8192].back = tmp;

  //Unlock the bucket
  omp_unset_lock(&hashtable.multihash[hval % hashtable.size].lock);
}

#endif
