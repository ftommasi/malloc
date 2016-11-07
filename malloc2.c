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


struct node head_node = {
  .next = NULL,
  .addr = 0,
  .size = 0,
  .free = 0,
};

struct node* head = & head_node;

size_t round(unsigned long to_round, unsigned int factor){
  return (size_t)(to_round + (factor-to_round % factor));
}

size_t round_to_page_size(unsigned long to_round){
  return round(to_round,sysconf(_SC_PAGE_SIZE));
}

size_t round_to_eight(unsigned long to_round){
  return round(to_round,8);
}


void split_node(struct node* current, size_t requested_size){
  struct node* second = ((char*)current->addr + requested_size);
  second->size = current->size -(requested_size + sizeof(struct node));
  second->addr = round_to_eight((char*)second + sizeof(struct node));
  second->free = 1;
  second->next = current->next;

  current->size = requested_size;
  current->next = second;
  current->free = 0;
}

void* TEST_malloc(size_t u_size){
  if(u_size < 1)
    return NULL;
  struct node* curr = head;
  while(curr->next){
    if(curr->free){
     
     if(curr->size > (u_size + 2*sizeof(struct node))){
       split_node(curr,u_size); 
       return curr->addr;
     } 
     else if(curr->size >= u_size){
       curr->free = 0;
       return curr->addr;
     }
    
    }
    curr = curr->next;
  }
  void* new = sbrk(round_to_page_size(u_size + sizeof(struct node)));
  if(new == (void*)-1){
    errno = ENOMEM;
    exit(-1);
  }
  
 struct  node* allocated;
 struct  node* remaining;

  allocated = (struct node*) new;
  allocated->addr = (void*)round_to_eight((char*)allocated + sizeof(struct node));
  allocated->next = NULL;
  allocated->free = 0;
  allocated->size = u_size;

  curr->next = allocated;
  long space_left = 
    round_to_page_size(u_size + sizeof(struct node)) 
    - u_size + 2* sizeof(struct node);
  if(space_left > sizeof(struct node)){
    remaining = (struct node*) ((char*)allocated->addr + u_size);
    remaining->addr = round_to_eight((char*)remaining + sizeof(struct node));
    remaining->free = 1;
    remaining->size = space_left - sizeof(struct node);
    remaining->next = NULL;

    allocated->next = remaining;
  }

  return allocated->addr;
}

void TEST_free(void* addr){
  struct node* curr = head;
  while(curr->next && curr->addr != addr){
    curr = curr->next;
  }

  if(curr->addr == addr){
    curr->free = 1;
  }
}
// --------- CHURN ------------------
#define NUMSLOTS 40

char *slots[NUMSLOTS];
size_t sizes[NUMSLOTS];

int churn_main(int argc, char** argv){

int i,n,size,maxblock;
  long j;
  int verbose=0;

  // Read arguments
  maxblock = -1;
  if (argc == 3) {
    if (!strcmp(argv[1],"-d")) {
      maxblock = atoi(argv[2]);
      verbose = 1;
    }
  }
  if (argc == 2)
    maxblock = atoi(argv[1]);

  if (maxblock <= 0) {
    fprintf(stderr,"usage: churn [-d] maxblock\n");
    exit(1);
  }

  // Initialize
  for (i = 0; i<NUMSLOTS; i++) slots[i] = NULL;
  srand(time(NULL));

  long mallocs=0,frees=0;  // number of mallocs, number of frees
  printf("mallocs\tfrees\tsbrk\n");

  // Main loop:
  while (1) {
    n = rand() % NUMSLOTS;
    
    if (slots[n]) {
      // Going to free slot[n]
      // Check contents to see if it's still 0,1,2,3,4,...
      for (j=1; j<sizes[n]; j++) {
	if (*(slots[n]+j) != (char) j) {
	  printf("Memory corruption detected.\n");
	  exit(1);
	}
      }
      
      if (verbose) printf("freeing slot %d.\n",n);
      TEST_free(slots[n]);
      slots[n] = NULL;
      frees++;

    } else {
      // Going to malloc slot[n]
      if (verbose) printf("malloc slot %d.\n",n);

      sizes[n] = rand() % maxblock + 1;
      slots[n] = (char *) TEST_malloc (sizes[n]);
      if (slots[n] == NULL) {
	fprintf(stderr,"out of memory\n");
	exit(1);
      }
      
      // Fill block with 0,1,2,3,4,...
      for (j=0; j<sizes[n]; j++) *(slots[n]+j) = (char) j;

      mallocs++;
    }
    
    // Periodically report progress
    if ((mallocs + frees) % 1000 == 0) 
      printf("%ld\t%ld\t%p\n",mallocs,frees,sbrk(0));
  }


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

  
