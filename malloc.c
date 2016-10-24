typedef struct node{
  node* next;
  void* addr;
  size_t size
  char free;
}node;

//head node 
node* head = &node;
head->next = NULL;
head->addr = NULL;
head->size = NULL;
head->free = NULL;

void* TEST_malloc(size_t size){
  node* curr = head;
  while(curr->next != NULL){
    //do shit here
    curr = curr->next;
  }

  size_t top = sbrk(0);
}

void* TEST_free(void* ptr){
node* curr = head;
  while(curr->next != NULL){
    //do shit here
    curr = curr->next;
  }

}
