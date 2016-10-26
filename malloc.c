#include <stdio.h>

struct node{
  struct node* next;
  void* addr;
  size_t size;
  char free;
}node;


//head node 
struct node mhead={
.addr = NULL,
.size = NULL,
.free = NULL,
.next = NULL 
};
struct node* head = &mhead;

void LLinsert(void* val){
 struct node* curr; 
  curr = head;
  while(curr->next){
    curr = curr->next;
  }
  
  struct node* ptemp =(struct node*) malloc(sizeof(struct node));
  ptemp-> addr = val;
  ptemp-> size = 1;
  ptemp-> free = 0;
  ptemp-> next = NULL;
  
  curr->next = ptemp;


}

void LLremove(){
  struct node* curr;
  struct node* prev;
  curr = head;
  while(curr->next){
    prev = curr;
    curr = curr->next;

  }
  printf("prev has val %d\n",prev->addr);
  printf("curr hasval %d\n",curr->addr);
  
  prev->next = NULL;
  free(curr);
}

void* TEST_malloc(size_t size){
  struct node* curr;
  curr = head;
  while(curr->next){
    //do shit here
    curr = curr->next;
  }

  size_t top = sbrk(0);
}

void* TEST_free(void* ptr){
  struct node* curr;
  curr = head;
  while(curr->next){
    //do shit here
    curr = curr->next;
  }

}


int main(){
  printf("Testing LinkedList\n");
  
  int i=0;
  for(i;i<10;i++){
    printf("inserting val %d\n",i);
    LLinsert(i);
  }
  
  struct node* curr;
  curr = head->next; 
  while(curr){
    printf("%d\n",curr->addr);
    curr = curr->next;
  }
  //printf("attempting to delete %d\n",curr->addr);  
  
  for(i=0;i<10;i++){
    LLremove();
  }
  printf("should be empty\n");
  curr = head->next; 
  while(curr){
    printf("%d\n",curr->addr);
    curr = curr->next;
  }
}
