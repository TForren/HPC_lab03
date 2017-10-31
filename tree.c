#include <stdlib.h>
#include <stdio.h>

struct p {
  int v;
  struct p * left;
  struct p * right;
};

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
  } else {
    if (v <= somewhere->v) {
      somewhere->left = add(v, somewhere->left);
    } else {
      somewhere->right = add(v, somewhere->right);
    }
    return(somewhere);
  }
}

struct p * find(int v, struct p * somewhere) {
  if (somewhere == NULL) {
    return NULL;
  } else {
    if (v == somewhere->v) {
      return somewhere;
    } else {
      if (v < somewhere->v) {
        find(v, somewhere->left);
      } else {
        find(v, somewhere->right);
      }
    }
  }
}

struct p * add_if_not_present(int v, struct p * somewhere);

int size();

int checkIntegrity();

int main() {
  struct p *testTree = newNode(50);

  add(25,testTree);
  add(15,testTree); 
  add(75,testTree);
  add(65,testTree);
  add(85,testTree);
  
  printf("right %d\n",testTree->right->v); 
  find(20, testTree);
  return 0;
}
