#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#define NUM_THREADS 16

struct timeval t0;
struct timeval t1;

static unsigned len = 2000000;
pthread_t tid[NUM_THREADS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct p {
  int v;
  struct p * left;
  struct p * right;
} p;

struct p *tree;

/* tree functions */
struct p * newNode(int value) {
  struct p * node = (struct p*)malloc(sizeof(struct p));
  
  node->v = value;
  node->left = NULL;
  node->right = NULL;
  return(node);
}

struct p * add(int v, struct p * somewhere) {
  
  if (somewhere == NULL) {
    return(newNode(v));
  } else if (v <= somewhere->v) {
    somewhere->left = add(v, somewhere->left);
  } else {
    somewhere->right = add(v, somewhere->right);
  }
  return(somewhere);
}

struct p * find(int v, struct p * somewhere) {
  if (somewhere == NULL) {
    return NULL;
  } else if (v == somewhere->v) {
    return somewhere;
  } else if (v < somewhere->v) {
    find(v, somewhere->left);
  } else {
    find(v, somewhere->right);
  }
}

struct p * add_if_not_present(int v, struct p * somewhere) { 
  if (somewhere == NULL) {
    return(newNode(v));
  } else if (v == somewhere->v) {
    return NULL;
  } else if (v < somewhere->v) {
    somewhere->left = add(v, somewhere->left);
  } else {
    somewhere->right = add(v, somewhere->right);
  }
  return(somewhere);
}

int size(struct p * somewhere) {
  int c = 1;
  if (somewhere == NULL) {
    return 0;
  } else {
    c += size(somewhere->left);
    c += size(somewhere->right);
    return c;
  }
}

int checkIntegrity(struct p * somewhere, int index, int nodeCount) {
  if (somewhere == NULL) {
    return(0);
  } else if (index >= nodeCount) {
    return(0);
  }

  return(checkIntegrity(somewhere->left, 2*index + 1, nodeCount) &&
	 checkIntegrity(somewhere->right, 2*index + 2, nodeCount));
}

/* workload functions */
void *baseline() {
  int i;
  for(i = 0; i<len;i++) {
  
    pthread_mutex_lock(&mutex);
  
    add(random(),tree); 

    add_if_not_present(random(),tree);
    
    pthread_mutex_unlock(&mutex);
  
  }
  pthread_exit(NULL);
}

void curFunc() {
   int i,err;
   for(i = 0; i < NUM_THREADS; i++) {
     err = pthread_create(&(tid[i]), NULL, &baseline, NULL); 
   };
   for(i = 0; i < NUM_THREADS; i++) {
     err = pthread_join(tid[i],NULL);
   };
};

/* All that PAPI Goodness */ 

int main() {
  
  long long values[1];
  int eventset;
   
  tree = (p *) malloc(len*sizeof(p)); 
  tree->v = random();
  gettimeofday(&t0,0);  
  curFunc();
  gettimeofday(&t1,0);
   
  double elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
  printf("time: %f\n", elapsed / 1000000 );  
  
  return 0;
}
