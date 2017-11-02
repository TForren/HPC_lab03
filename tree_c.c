#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <pthread.h>
#define NUM_THREADS 16

int i;
static unsigned len = 16*(1 << 20);
pthread_t tid[NUM_THREADS];
int lock = 0;

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
  for(i = 0; i<len;i++) {

    while (lock); 
    
    lock = 1;
    
    add(random(),tree);  

    add_if_not_present(random(),tree);
    
    lock = 0;
  }
}

void curFunc() {
   int err;
   for(i = 0; i < NUM_THREADS; i++) {
   	err = pthread_create(&(tid[i]), NULL, &baseline, NULL); 
   //test(tree);
   //baseline(tree);
   //seq(tree);
  };
};

/* All that PAPI Goodness */ 

int events[1] = {PAPI_L2_DCM};
int eventnum = 1;

int main() {
/*  struct p *testTree = newNode(50);

  add(25,testTree);
  add(15,testTree); 
  add(75,testTree);
  add(65,testTree);
  add(85,testTree);
  
  printf("right %d\n",testTree->right->v); 
  find(20, testTree);
*/

  long long values[1];
  int eventset;

  if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT)) {
    printf("Can't initiate PAPI library!\n");
    exit(-1);
  }

  eventset = PAPI_NULL;
  if(PAPI_create_eventset(&eventset) != PAPI_OK) {
    printf("can't create eventset!\n");
    exit(-3);
  }

  if(PAPI_OK != PAPI_add_events(eventset, events, eventnum)) {
    printf("can't add events!\n");
    exit(-4);
  }
 
  tree = (p *) malloc(len*sizeof(p)); 
  PAPI_start(eventset);
  curFunc();
  PAPI_stop(eventset,values);
  free(tree);
 
  char event_name[PAPI_MAX_STR_LEN];
  if (PAPI_event_code_to_name( events[0], event_name ) == PAPI_OK)
	printf("%s: %lld\n", event_name, values[0]);
 
  return 0;
}
