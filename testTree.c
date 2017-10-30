#include <stdio.h>
#include <stdlib.h>

struct node {
  int data;
  struct node* left;
  struct node* right;
};

struct node* NewNode(int data) {
  struct node* node = (struct node*)malloc(sizeof(struct node)); 
  node->data = data;
  node->left = NULL;
  node->right = NULL;

  return(node);
}


struct node* insert(struct node* node, int data) {
  if (node == NULL) {
    return(NewNode(data));
  }
  else {
    if (data <= node->data) node->left = insert(node->left, data);
    else node->right = insert(node->right, data);

    return(node);
  }
}


int main(void) {

  struct node *root = NewNode(50);

  insert(root, 25);
  insert(root, 15);
  insert(root, 35);

  printf("%d\n", (root->left->data));
  printf("%d\n", (root->left->left->data));


  return(0);
}
