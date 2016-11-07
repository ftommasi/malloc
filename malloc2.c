#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct node{
  struct node* next;
  void* addr;
  size_t size;
  char free;
}node;

#define node struct node

node head_node = {
  .next = NULL,
  .addr = 0,
  .size = 0,
  .free = 0,
};
node* head = & head_node;

size_t round(unsigned long to_round, unsigned int factor){
  return (size_t)to_round + (factor-to_round % factor);
}

size_t round_to_page_size(unsigned long to_round){
  return round(to_round,sysconf(_SC_PAGE_SIZE));
}

size_t round_to_eight(unsigned long to_round){
  return round(to_round,8);
}


void split_node(node* current, size_t requested_size){
  node* second = ((char*)current->addr + requested_size);
  
}

void* TEST_malloc(size_t u_size){
  node* curr = head;
  while(curr->next){
    curr = curr->next;
    if(curr->free){
     
     if(curr->size > (u_size + 2*sizeof(node))){
       split_node(curr,u_size); 
       return curr->addr;
     } 
     else if(curr->size >= u_size){
       curr->free = 0;
       return curr->addr;
     }


    
    }

  }
  void* new = sbrk(round_to_page_size(u_size + sizeof(node)));
  if(new == (void*)-1){
    errno = ENOMEM;
    exit(-1);
  }
  
  node* allocated;
  node* remaining;

  allocated = (node*) new;
  allocated->addr = (void*)round_to_eight((char*)allocated + sizeof(node));
  allocated->next = NULL;
  allocated->free = 0;
  allocated->size = u_size;

  curr->next = allocated;
  int space_left = 0;
  if(space_left > sizeof(node)){
    remaining = (node*) ((char*)allocated->addr + u_size);
    remaining->addr = round_to_eight((char*)remaining + sizeof(node));
    remaining->free = 1;
    remaining->size = space_left - sizeof(node);
    remaining->next = NULL;

    allocated->next = remaining;
  }

  return allocated->addr;
}

void TEST_free(void* addr){
  node* curr = head;
  while(curr->next && curr->addr != addr){
    curr = curr->next;
  }

  if(curr->addr == addr){
    curr->free = 0;
  }
}
// --------- CHURN ------------------
int churn_main(int argc, char** argv){

}

//---------------------------------
//
//--------------CHURN2---------------
int churn_2_main(int argc,char** argv){

}
///---------------------------------
int main(int argc, char** argv){

  churn_main(argc,argv);
  //churn_2_main(argc,argv);
}
