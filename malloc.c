#include <stdio.h>

struct node{
  struct node* next;
  void* addr;
  size_t size;
  char free;
}node;


//head node 
struct node mhead;
/*
mhead.next = NULL;
mhead.addr = NULL;
mhead.size = NULL;
mhead.free = NULL;
*/
struct node* head = &head;

void LLinsert(struct node* node){
  struct node* curr;
  curr = head;
  while(curr->next != NULL){
    //do shit here
    curr = curr->next;
  }

  curr->next = node;
}

void LLremove(){
  struct node* curr;
  curr = head;
  while(curr->next != NULL){
    //do shit here
    curr = curr->next;
  }
  
  struct node* temp;
  temp = curr;
  curr->next = NULL;
  free(temp);
}

void* TEST_malloc(size_t size){
  struct node* curr;
  curr = head;
  while(curr->next != NULL){
    //do shit here
    curr = curr->next;
  }

  size_t top = sbrk(0);
}

void* TEST_free(void* ptr){
  struct node* curr;
  curr = head;
  while(curr->next != NULL){
    //do shit here
    curr = curr->next;
  }

}


int main(){
  printf("Testing LinkedList\n");
  int i=0;
  for(i;i<10;i++){
    struct node temp;
    temp.addr = i;
    temp.size = 1;
    temp.free = 0;
    LLinsert(&temp);
  }
  
  struct node* curr;
  curr = head;
  while(curr->next != NULL){
    printf("%d\n",curr->addr);
    curr = curr->next;
  }
}
