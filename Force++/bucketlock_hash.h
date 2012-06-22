#ifndef _HASH_H
#define _HASH_H
#include <omp.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <crypt.h>
#include <stdlib.h>
#include "sha1.h"
#include <omp.h>

using namespace std;

template<class V>
class bucketlock_hash {
  private:
    typedef struct {
      omp_lock_t lock;
      unordered_map<string,V> tree;
    } bucketType;

    unsigned long hash(string);
    vector<bucketType> buckets;

  public:
    bucketlock_hash(unsigned int);
    ~bucketlock_hash();

    bool get(string, V&);
    void put(string,V);

    bool find(string);
    unsigned int size();
};

template<class V>
unsigned long bucketlock_hash<V>::hash(string key) {
  unsigned char hash[20];
  unsigned long hash_int = 0;

  sha1::calc((unsigned char*)key.c_str(), key.size(), hash);
  for(int i=0; i<5; i++) hash_int += (int)hash[i]*(256^4-i);

  return hash_int;
}

template<class V>
bucketlock_hash<V>::bucketlock_hash(unsigned int size) {
  buckets.resize(size);
  for(unsigned int i=0; i<size; i++) 
    omp_init_lock(&buckets[i].lock);
}

template<class V>
bucketlock_hash<V>::~bucketlock_hash() {
  for(unsigned int i=0; i<buckets.size(); i++)
    omp_destroy_lock(&buckets[i].lock);
}

template<class V>
bool bucketlock_hash<V>::get(string key, V &value) {
  /* hash the key to determine the bucket */
  unsigned long bucketID = hash(key) % buckets.size();

  if(buckets[bucketID].tree.find(key) != buckets[bucketID].tree.end()) {
    value = buckets[bucketID].tree[key];
    return true;
  } else return false;

}

template<class V>
void bucketlock_hash<V>::put(string key, V value) {
  /* hash the key to determine the bucket */
  unsigned long bucketID = hash(key) % buckets.size(); 

  //Lock the bucket to prevent others from writing to it
  omp_set_lock(&buckets[bucketID].lock);
  buckets[bucketID].tree[key] = value;
  //Unlock the bucket to allow others to write
  omp_unset_lock(&buckets[bucketID].lock);
}

template<class V>
bool bucketlock_hash<V>::find(string key) {
  V tmp;
  return get(key, tmp);
}

#endif
