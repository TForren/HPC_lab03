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
  printf("adding %d\n",v);
  
  if (somewhere == NULL) {
    printf("null. new node\n");
    return(newNode(v));
  } else {
    if (v <= somewhere->v) {
      printf("less than current v. left of %d\n",somewhere->v);
      somewhere->left = add(v, somewhere->left);
    } else {
      printf("greater than current v. right\n");
      somewhere->right = add(v, somewhere->right);

      return(somewhere);
    }
  }
}

struct p * find(int v);

struct p * add_if_not_present(int v, struct p * somewhere);

int size();

int checkIntegrity();

int main() {
  struct p *testTree = newNode(50);

  add(25,testTree);
  printf("left %d\n",testTree->left->v); 
  add(15,testTree);
 // add(35,testTree);
  printf("root %d\n",testTree->v);
  printf("left %d\n",testTree->left->v); 
  
  //add(75,testTree);
  //add(65,testTree);
  //add(85,testTree);

  //printf("right %d\n",testTree->left->right->v); 
  return 0;
}
