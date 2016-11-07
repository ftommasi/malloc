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

size_t round(unsigned long to_round, unsigned long factor){
  return (size_t)(to_round + (factor-to_round % factor));
}

size_t round_to_page_size(unsigned long to_round){
  return round(to_round,sysconf(_SC_PAGE_SIZE));
}

size_t round_to_eight(unsigned long to_round){
  return round(to_round,8);
}


void split_node(struct node* current, size_t requested_size){
  struct node* second = (struct node*)((char*)current->addr + requested_size);
  second->size = current->size -(requested_size + sizeof(struct node));
  second->addr = (void*)round_to_eight((unsigned long)((char*)second + sizeof(struct node)));
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
  void* new = sbrk(round_to_page_size((unsigned long)(u_size + sizeof(struct node))));
  if(new == (void*)-1){
    errno = ENOMEM;
    exit(-1);
  }
  
 struct  node* allocated;
 struct  node* remaining;

  allocated = (struct node*) new;
  allocated->addr = (void*)round_to_eight(
      (unsigned long)((char*)allocated + sizeof(struct node))
      );
  allocated->next = NULL;
  allocated->free = 0;
  allocated->size = u_size;

  curr->next = allocated;
  long space_left = 
    round_to_page_size((unsigned long)
        (u_size + sizeof(struct node))) 
    - u_size + 2* sizeof(struct node);
  if(space_left > sizeof(struct node)){
    remaining = (struct node*) ((char*)allocated->addr + u_size);
    remaining->addr = (void*)round_to_eight(
       (unsigned long)((char*)remaining + sizeof(struct node))
       );
    remaining->free = 1;
    remaining->size = space_left - sizeof(struct node);
    remaining->next = NULL;

    allocated->next = remaining;
  }

  return allocated->addr;
}


void TEST_free(void* addr){
  if(addr){
    struct node* curr = head;
    while(curr->next && curr->addr != addr){
      curr = curr->next;
    }

    if(curr->addr == addr){
      curr->free = 1;
    }
  }
}


void* TEST_calloc(size_t nmemb, size_t size){
  void* array = TEST_malloc(nmemb*size);
  memset(array,0,nmemb*size);
  return array;
}


void* TEST_realloc(void *ptr, size_t size){
  struct node* curr = head;
  while(curr->next && curr->addr != ptr){
    curr = curr->next;
  }
  if(curr->next == ptr){
    if(curr->size >= size){
      curr->free = 0;
      return curr->addr;
    }
    else{
      void* new = TEST_malloc(size);
      memcpy(new,ptr,curr->size);
      TEST_free(ptr);
      return new;
    }
  }
  return NULL;
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


#define NUMSLOTS2 32
long *slots2[NUMSLOTS2];
int sizes2[NUMSLOTS2];
#define CHURNS 2000

int churn_2_main(int argc,char** argv){

  int i,j,n,size,maxblock;
  int c=0, f=0, r=0;
  int d=0;
  int cc=0;

  maxblock = -1;
  if (argc == 3) {
    if (!strcmp(argv[1],"-d")) {
      maxblock = atoi(argv[2]);
      d = 1;
    }
  }
  if (argc == 2)
    maxblock = atoi(argv[1]);

  if (maxblock <= 0) {
    fprintf(stderr,"usage: churn2 [-d] maxblock\n");
    exit(1);
  }

  for (i = 0; i<NUMSLOTS2; i++) slots2[i] = NULL;
  srand(time(NULL));
  
  printf("calloc\trealloc\tfree\tsbrk\n");

  while (cc < CHURNS) {
    for (i = 0; i < NUMSLOTS2 * NUMSLOTS2; i++) {
      n = rand() % NUMSLOTS2;

      if (slots2[n]) {
	/* Slot is full */
	if (d) printf("Testing slot %d...",n);

	/* Check contents */
	for (j=1; j<sizes2[n]; j++) {
	  if (*(slots2[n]+j) != j) {
	    printf("Memory error detected.\n");
	    exit(1);
	  }
	}
	
	if (d) printf("ok\n");

	/* Free or realloc at random */
	if (rand() % 2) {
	  /* Free it */
	  if (d) printf("Freeing slot %d.\n",n);
	  free(slots2[n]);
	  slots2[n] = NULL;
	  f++;

	} else {
	  /* Reallocate it */
	  if (d) printf("Realloc slot %d...",n);
	  size = rand() % maxblock + 1;
	  slots2[n] = (long *) realloc(slots2[n],size*sizeof(long));
	  if (slots2[n] == NULL) {
	    fprintf(stderr,"realloc: out of memory\n");
	    exit(1);
	  }
	  if (sizes2[n] < size) {
	    if (d) printf("larger.\n");
	    for (j = sizes2[n]; j<size; j++) {
	      *(slots2[n]+j) = j;
	    }
	  } else {
	    if (d) printf("smaller.\n");
	  }
	  sizes2[n] = size;

	  r++;

	}

      } else {
	/* Slot is empty */
	if (d) printf("Calloc  slot %d.\n",n);

	sizes2[n] = rand() % maxblock + 1;
	slots2[n] = (long *) calloc (sizes2[n],sizeof(long));

	if (slots2[n] == NULL) {
	  fprintf(stderr,"calloc: out of memory\n");
	  exit(1);
	}

	/* Fill block with 1,2,3,4,... */
	for (j=0; j<sizes2[n]; j++) {
	  if (*(slots2[n]+j)) {
	    printf("Calloc failed to zero memory\n");
	    exit(1);
	  }
	  *(slots2[n]+j) = j;
	}

	c++;
      }

    }
    printf("%d\t%d\t%d\t%p\n",c,r,f,sbrk(0));
    c = 0; f = 0; r=0;
    cc++;
  }

}
///---------------------------------


int main(int argc, char** argv){

  churn_main(argc,argv);
  churn_2_main(argc,argv);
}

  


