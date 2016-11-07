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

//head node to Linked List 
struct node mhead={
.addr = 0,
.size = 0,
.free = 0,
.tail = 1,
.next = 0 
};
struct node* head = &mhead;

void my_print(char* string,int size){
  write(STDOUT_FILENO,string,size);
}

size_t round_to_nearest_pagesize(size_t size){
  long page_size = sysconf(_SC_PAGESIZE);
  return (size_t)size + (page_size - size % page_size);
}

size_t round_to_nearest_eight(long  i){
  return (size_t)i + (8 - i % 8);
}

void* TEST_malloc(size_t size){
  if(size <= 0)
    return NULL;
  void* top = sbrk(0);
  struct node* curr;
  curr = head;
 
  while(curr->next){
#if 0 
  if(curr->free && curr->size >= size){
    printf("PLUG AND CHUG BABY\n");
    curr->free = 0;
    return curr->addr; 
  }
#endif

#if  1 
 if(curr->free && curr->size > (size + 2*sizeof(struct node))){
   
   //if there is room for two non-zero size'd nodes
   if(curr->size - (size+sizeof(node)) > 0){
      printf("--SPLITTING NODE--\n");
           struct node* second_node;
      //curr becomes the first half of split node
      second_node = (struct node*)((char*)curr->addr + size);
      second_node->size = curr->size - (size + sizeof(struct node));
      second_node->addr = (void*)
        round_to_nearest_eight(((char*)second_node + sizeof(node)));
      second_node->tail = curr->tail;
      second_node->free = 1;
      second_node->next = curr->next;
     
      curr->size = size;
      curr->free = 0;
      curr->tail = 0;
      curr->next = second_node;
      
      return curr->addr;  
    }
   }
   
 //if there is only room for one 
    if(curr->free && curr->size >= size){
      printf("PLUG AND CHUG BABY\n");
      
      curr->free = 0;
      return curr->addr; 
     }
#endif
    curr = curr->next;
  }
  

    printf(":::Call to SBRK:::\n");
  void* new = 
    sbrk(round_to_nearest_pagesize(size+sizeof(struct node)));
  if( new == (void*)-1 ){
    printf("Error calling sbrk!\n");
    errno = ENOMEM;
    exit(-1);
    //return NULL;
  }
  //currently the node* is at begginning of memory chunk
  struct node* n;
  struct node* free;
  n = (struct node*) new;
  //the free space is just the memory after the node and the requested size at the address given by sbrk
  curr->next = n;
  n->free = 0;
  n->size = size;
  n->addr = (void*)round_to_nearest_eight((char*)new+sizeof(node));
  n->tail= 0;
  n->next = NULL;
#if 1
  //if (sysconf(_SC_PAGESIZE) - (size+sizeof(struct node)) > 0 ){
  if (
      round_to_nearest_pagesize(size+sizeof(struct node))
      > 
      (size + 2*sizeof(struct node))
      ){
    free = (struct node*)((char*) n->addr +  size);
    n->next = free;
    free->addr= (void*)round_to_nearest_eight(
        (char*)free + sizeof(struct node));
    free->free = 1;
    free->size = 
      round_to_nearest_pagesize(size+sizeof(struct node)) - 
      (size + 2*sizeof(struct node));                                 
    free->next = (struct node*)"FUCK";
    free->tail = 0;
  }
#endif
  return n->addr;
  }


  void TEST_free(void* ptr){
  struct node* curr;
  curr = head;

  while(curr->next && curr->addr != ptr){
    //printf("want to free %p | looking at %p (addr %p)\n",ptr,curr,curr->addr);
    curr = curr->next;
  }
  if(curr->addr == ptr){
   printf("FREE'd %p\n",curr->addr);
    curr->free = 1;
  }

}



#define NUMSLOTS 10
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
    printf("[n is now %d]\n",n) ;
    if (slots[n]) {
      // Going to free slot[n]
      // Check contents to see if it's still 0,1,2,3,4,...
      
      if (verbose) printf("freeing slot %d.\n",n);
      for (j=1; j<sizes[n]; j++) {
	      if (*(slots[n]+j) != (char) j) {
      	  printf("Memory corruption detected.\n");
      	  exit(1);
      	}
      }
      
      TEST_free(slots[n]);
      slots[n] = NULL;
      frees++;

      printf("Memory Validated for slot %d.\n",n);
    }
    
    else {
      // Going to malloc slot[n]
      if (verbose) printf("malloc slot %d.\n",n);
     
      sizes[n] = rand() % maxblock + 1;
      if(verbose) printf("Malloc'ing %d\n",sizes[n]); 
      slots[n] = (char *) TEST_malloc (sizes[n]);
      if (slots[n] == NULL) {
	      fprintf(stderr,"out of memory\n");
	      exit(1);
      }
      
      // Fill block with 0,1,2,3,4,...
      for (j=0; j<sizes[n]; j++){ 
        
        *(slots[n]+j) = (char) j;
            

     //   if(verbose) 
//        printf("MALLOC'd %s (n=%d,j=%d) AT %p\n" ,*(slots[n]+j),n,j,&slots[n]+j);
        
      }
      //validating what I just did
      for (j=1; j<sizes[n]; j++) {
        if (*(slots[n]+j) != (char) j){
          printf("Memory corruption detected.\n");
          exit(1);
        }
      }
      
      printf("malloc successful\n");
      mallocs++;
    }
    
    // Periodically report progress
    if ((mallocs + frees) % 1000 == 0) 
      printf("%ld\t%ld\t%p\n",mallocs,frees,sbrk(0));
  }
}


int main(int argc, char** argv){

 churn_main(argc,argv);
 
 
 void* c = TEST_malloc(5000);
 printf("my node %d\n",sizeof(node));
 
 c = "TEST STRING";
 TEST_free(c);
 void* p = TEST_malloc(1000);
 p = "TEST SSTRING 2\n";
 TEST_free(p);
 void* a = TEST_malloc(1000);
 a = "TEST STRING 3\n";
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
