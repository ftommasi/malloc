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
  char tail;
  char free;
}node;

//head node 
struct node mhead={
.addr = 0,
.size = 0,
.free = 0,
.tail = 1,
.next = 0 
};
struct node* head = &mhead;

long  round_to_nearest_pagesize(size_t size){
  long page_size = sysconf(_SC_PAGESIZE);
  return size + (size % page_size);
}

long  round_to_nearest_eight(long  i){
  return i + (8 - i % 8);
}

void* TEST_malloc(size_t size){
  if(size <= 0)
    return NULL;
  void* top = sbrk(0);
  struct node* curr;
  curr = head;
  while(!curr->tail){
    //if(curr->free){} //split the node
    curr = curr->next;
  }
  
  void* new = 
    sbrk(round_to_nearest_pagesize(size+sizeof(node)));
  if( new == (void*)-1 ){
    printf("Error calling sbrk!\n");
    errno = ENOMEM;
    exit(-1);
    //return NULL;
  }
  
  
  //currently the node* is at begginning of memory chunk
  struct node* n;
  n = (struct node*) new;
  curr->next = n;
  n->size = size;
  n->addr = (void*)round_to_nearest_eight(new);
  n->tail= 1;
  n->next = NULL;
  //return new;
  return (void*)round_to_nearest_eight(new+sizeof(node));
}

void* TEST_free(void* ptr){
  struct node* curr;
  curr = head;
  while(curr->next){
    if(curr == ptr) break;
    curr = curr->next;
  }
  if(!curr->free) 
    curr->free = 1;

}



#define NUMSLOTS 100
char *slots[NUMSLOTS];
size_t sizes[NUMSLOTS];

int main(int argc, char** argv){
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
            
        if (verbose) printf("freeing slot %d.\n",n);
        
        TEST_free(slots[n]);
        slots[n] = NULL;
        frees++;
      
      }
    }
    
    else {
      // Going to malloc slot[n]
      if (verbose) printf("malloc slot %d.\n",n);

      sizes[n] = rand() % maxblock + 1;
      slots[n] = (char *) TEST_malloc (sizes[n]);
      if (slots[n] == NULL) {
	      fprintf(stderr,"out of memory\n");
	      exit(1);
      }
      
      if(verbose) printf("MALLOC'd AT %p\n",&slots[n]);
      // Fill block with 0,1,2,3,4,...
      for (j=0; j<sizes[n]; j++) *(slots[n]+j) = (char) j;

      mallocs++;
    }
    
    // Periodically report progress
    if ((mallocs + frees) % 1000 == 0) 
      printf("%ld\t%ld\t%p\n",mallocs,frees,sbrk(0));
}
}
//--------------------------------------

/*
 * churn.c
 *
 * usage: churn [-v] maxblock\n");
 * 
 *    Allocate & free memory
 *
 *    Tests malloc() and free() by repeatedly allocating and freeing
 *    chunks of memory of up to size maxblock.
 *
 *    -v is verbose mode, and prints a message with each call to malloc/free
 *
 * Bryan Clair 2002-2013
 * v2.0 2010: churn now actually writes data into the memory it's allocated, and checks
 *            that the data survives.
// At any time, churn may have up to NUMSLOTS memory allocations
#define NUMSLOTS 40

char *slots[NUMSLOTS];
size_t sizes[NUMSLOTS];

main(int argc, char *argv[])
{
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
      free(slots[n]);
      slots[n] = NULL;
      frees++;

    } else {
      // Going to malloc slot[n]
      if (verbose) printf("malloc slot %d.\n",n);

      sizes[n] = rand() % maxblock + 1;
      slots[n] = (char *) malloc (sizes[n]);
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

*/
