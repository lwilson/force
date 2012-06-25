typedef struct {
  typedef struct {
    int size;
    long *inner;
  } innerlist;

  int size;
  innerlist *outer;
} cart_set;

long *getCartProd(int index, cart_set myVecs, int *num) {
  long *result;
  int i;

  *num = myVecs.size;
  result = (long*)malloc(sizeof(long)*(*num));

  for(i=(*num)-1; i>0; i--) {
    result[i] = myVecs.outer[i].inner[index % myVecs.outer[i].size];
    index /= myVecs.outer[i].size;
  }

  if(index >= myVecs.outer[0].size) {
    free(result);
    result = NULL;
    *num = 0;
  } else {
    result[0] = myVecs.outer[0].inner[index % myVecs.outer[0].size];
  }

  return result;
}

unsigned long sizeofCartProd(cart_set myVecs) {
  unsigned long size = 1;
  int i;

  if(!myVecs.size) return 0;
  for(i=0; i<myVecs.size; i++) size *= myVecs.outer[i].size;
  return size;
}
