#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#define NUM_THREADS 16

struct timeval t0;
struct timeval t1;

static unsigned len = 2000000;
pthread_t tid[NUM_THREADS];
pthread_mutex_t full_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t left_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t right_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct p {
  int v;
  struct p * left;
  struct p * right;
  pthread_mutex_t mutex;
} p;

struct p *tree;

/* tree functions */
struct p * newNode(int value, struct p * parent) {
  struct p * node = (struct p*)malloc(sizeof(struct p));
  node->v = value;
  node->left = NULL;
  node->right = NULL;
  return(node);
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


struct p * add(int v, struct p * somewhere, struct p * parent) {
  if (somewhere == NULL) {
    return(newNode(v,parent));
  } else if (v <= somewhere->v) {
    somewhere->left = add(v, somewhere->left, somewhere);
  } else {
    somewhere->right = add(v, somewhere->right, somewhere);
  }
  return(somewhere);
}


struct p * addRoot(int v, struct p * somewhere, struct p * parent) {
  //printf("%d\n", v);
  if (somewhere == NULL) {
    return(newNode(v,parent));
  } else if (v <= somewhere->v) {
    pthread_mutex_lock(&left_mutex);
    somewhere->left = add(v, somewhere->left, somewhere);
    pthread_mutex_unlock(&left_mutex);
  } else {
    pthread_mutex_lock(&right_mutex);
    somewhere->right = add(v, somewhere->right, somewhere);
    pthread_mutex_unlock(&right_mutex);
  }
  return(somewhere);
}


struct p * add2(int v, struct p * somewhere, struct p * parent) {
  
  if (somewhere == NULL) {
    //printf("tmp\n");
    pthread_mutex_unlock(&parent->mutex);
    return(newNode(v,parent));
  } else {
    pthread_mutex_lock(&somewhere->mutex);
    if (somewhere != parent) 
    	pthread_mutex_unlock(&parent->mutex);
    if (v <= somewhere->v) {
      somewhere->left = add2(v, somewhere->left, somewhere);
    } else {
      somewhere->right = add2(v, somewhere->right, somewhere);
    }
  }
  return(somewhere);
}


struct p * add_if_not_present(int v, struct p * somewhere, struct p * parent) { 
  if (somewhere == NULL) {
    return(newNode(v,parent));
  } else if (v == somewhere->v) {
      return NULL;
  } else {
    if (v <= somewhere->v) {
      somewhere->left = add(v, somewhere->left, somewhere);
    } else {
      somewhere->right = add(v, somewhere->right, somewhere);
    }
  }
  return(somewhere);
}

struct p * add_if_not_present2(int v, struct p * somewhere, struct p * parent) { 
  if (somewhere == NULL) {
    //printf("fef\n");
    pthread_mutex_unlock(&parent->mutex);    
    return(newNode(v,parent));
  } else if (v == somewhere->v) {
      pthread_mutex_unlock(&parent->mutex);
      return NULL;
  } else {
    pthread_mutex_lock(&somewhere->mutex);
    if (somewhere != parent) 
    	pthread_mutex_unlock(&parent->mutex);
    if (v <= somewhere->v) {
      somewhere->left = add_if_not_present2(v, somewhere->left, somewhere);
    } else {
      somewhere->right = add_if_not_present2(v, somewhere->right, somewhere);
    }
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
    return(1);
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
  
    pthread_mutex_lock(&full_mutex);

    add(random(),tree,tree); 

    add_if_not_present(random(),tree,tree);
    
    pthread_mutex_unlock(&full_mutex);
  }
}


void *better() {
  int i;
  for(i = 0; i<len; i++) {
     
    addRoot(random(),tree,tree);
    
    add_if_not_present(random(),tree,tree);

  }
}

void curFunc() {
   int i;
   int err;
   for(i = 0; i < NUM_THREADS; i++) {
   	err = pthread_create(&(tid[i]), NULL, &better, NULL); 
   };
   for(i = 0; i < NUM_THREADS; i++) {
   	pthread_join(tid[i],NULL);
   };
};

/* All that PAPI Goodness */ 

int main() {
  
  tree = (p *) malloc(len*sizeof(p)); 
  tree->v = random();
  gettimeofday(&t0,0);
  curFunc();
  gettimeofday(&t1,0);
  double elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;
  printf("time: %f\n", elapsed/ 1000000 );
 
  return 0;
}
