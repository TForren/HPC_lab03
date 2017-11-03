#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <pthread.h>
#define NUM_THREADS 16

static unsigned len = 16*(1 << 5 );
pthread_t tid[NUM_THREADS];
pthread_mutex_t full_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t left_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t right_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct p {
  int v;
  struct p * left;
  struct p * right;
  struct p * parentNode;
  pthread_mutex_t mutex;
  //pthread_mutex_t left;
  //pthread_mutex_t right;
} p;

struct p *tree;

/* tree functions */
struct p * newNode(int value, struct p * parent) {
  struct p * node = (struct p*)malloc(sizeof(struct p));
  pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
  node->mutex = tmp;
  node->v = value;
  node->left = NULL;
  node->right = NULL;
  node->parentNode = parent;
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


struct p * add2(int v, struct p * somewhere, struct p * parent) {
  
  if (somewhere == NULL) {
    printf("tmp\n");
    return(newNode(v,parent));
  } else {
    pthread_mutex_lock(&somewhere->mutex);
    pthread_mutex_unlock(&parent->mutex);
    if (v <= somewhere->v) {
      printf("left add\n");
      somewhere->left = add2(v, somewhere->left, somewhere);
    } else {
      printf("right add\n");
      somewhere->right = add2(v, somewhere->right, somewhere);
    }
  }
  return(somewhere);
}


struct p * add3(int v, struct p * somewhere, struct p * parent) {

  if (somewhere == NULL) {
    return(newNode(v,parent));
  } else {
    if (v <= somewhere->v) {
      if (somewhere->left == NULL) { 
        pthread_mutex_lock(&somewhere->mutex);
        somewhere->left = add3(v, somewhere->left, somewhere);
	pthread_mutex_unlock(&parent->mutex);
      } else {
        somewhere->left = add3(v, somewhere->left, somewhere);
      }	
    } else {
      if (somewhere->right == NULL) { 
        pthread_mutex_lock(&somewhere->mutex);
        somewhere->right = add3(v, somewhere->right, somewhere);
        pthread_mutex_unlock(&parent->mutex);
      } else { 
	somewhere->right = add3(v, somewhere->right, somewhere);
      }
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
    if (v < somewhere->v) {
      somewhere->left = add_if_not_present(v, somewhere->left, somewhere);
    } else {
      somewhere->right = add_if_not_present(v, somewhere->right, somewhere);
    }
  }
  return(somewhere);
}

struct p * add_if_not_present2(int v, struct p * somewhere, struct p * parent) { 
  if (somewhere == NULL) {
    return(newNode(v,parent));
  } else if (v == somewhere->v) {
      return NULL;
  } else {
    pthread_mutex_lock(&somewhere->mutex);
    pthread_mutex_unlock(&parent->mutex);
    if (v < somewhere->v) {
      somewhere->left = add_if_not_present2(v, somewhere->left, somewhere);
    } else {
      somewhere->right = add_if_not_present2(v, somewhere->right, somewhere);
    }
  }
  return(somewhere);
}


struct p * add_if_not_present3(int v, struct p * somewhere, struct p * parent) { 
  if (somewhere == NULL) {
    return(newNode(v,parent));
  } else if (v == somewhere->v) {
      return NULL;
  } else {
    if (v < somewhere->v) {
      if (somewhere->left == NULL) { 
        pthread_mutex_lock(&somewhere->mutex);
        somewhere->left = add_if_not_present3(v, somewhere->left, somewhere);
	pthread_mutex_unlock(&parent->mutex);
      } else {
        somewhere->left = add_if_not_present3(v, somewhere->left, somewhere);
      }	
    } else {
      if (somewhere->right == NULL) { 
        pthread_mutex_lock(&somewhere->mutex);
        somewhere->right = add_if_not_present3(v, somewhere->right, somewhere);
        pthread_mutex_unlock(&parent->mutex);
      } else { 
	somewhere->right = add_if_not_present3(v, somewhere->right, somewhere);
      }
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

void *meh() {
  int i;
  for(i = 0; i < len; i++) {
    
    add2(random(),tree,tree);

    //add_if_not_present2(random(),tree,tree);

  }
}

void *better() {
  int i;
  for(i = 0; i<len; i++) {
     
    add3(random(),tree,tree);
    
    add_if_not_present3(random(),tree,tree);

  }
}

void curFunc() {
   int i;
   int err;
   for(i = 0; i < NUM_THREADS; i++) {
   	err = pthread_create(&(tid[i]), NULL, &meh, NULL); 
   };
   for(i = 0; i < NUM_THREADS; i++) {
   	pthread_join(tid[i],NULL);
   };
};

/* All that PAPI Goodness */ 

int events[1] = {PAPI_L2_DCM};
int eventnum = 1;

int main() {
  
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
  clock_t clock_start = clock();
  curFunc();
  clock_t clock_end = clock();
  PAPI_stop(eventset,values);
  printf("time: %f\n", ((double)(clock_end - clock_start)) / CLOCKS_PER_SEC);
 
  char event_name[PAPI_MAX_STR_LEN];
  if (PAPI_event_code_to_name( events[0], event_name ) == PAPI_OK)
	printf("%s: %lld\n", event_name, values[0]);
 
  return 0;
}
