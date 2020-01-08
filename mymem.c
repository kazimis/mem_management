#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */
struct memoryList
{
  // doubly-linked list
  struct memoryList *prev;
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};


strategies myStrategy = NotSet;    // Current strategy
size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;



/*function for memory allocation policies to update
  memory after memory allocation
*/
void update(struct memoryList* mem_block, size_t requested){
  mem_block->alloc = 1;
  if(mem_block->size > requested){
    struct memoryList* remainder_mem = malloc(sizeof(struct memoryList));
    if(mem_block -> next){
      remainder_mem->next = mem_block->next;
      mem_block->next->prev = remainder_mem;
    }else{
      remainder_mem->next = NULL;
    }
    remainder_mem->prev = mem_block;
    mem_block->next = remainder_mem;

    //update size and alloc and location in memory pool
    remainder_mem->size = mem_block->size - requested;
    remainder_mem->alloc = 0;
    remainder_mem->ptr = mem_block->ptr + requested;
    mem_block->size = requested;
    next = remainder_mem;
  }else {
    if(mem_block->next){
      next = mem_block->next;
    }else{
      next = head;
    }
  }
}//end of update



/*function to join the current block with left empty block*/
void join_left(struct memoryList* current_block){
  struct memoryList* prev_block = current_block->prev;

  prev_block->next = current_block->next;
  if(current_block->next){
    current_block->next->prev = prev_block;
  }
  prev_block->size += current_block->size;

  if(next == current_block) {
    next = prev_block;
  }
  free(current_block);
}


/*function to join the current block with right empty block*/
void join_right(struct memoryList* current_block){
  struct memoryList* next_block = current_block->next;

  current_block->next = next_block->next;
  if(next_block->next){
    next_block->next->prev = current_block;
  }
  current_block->size += next_block->size;
  if(next == next_block) {
    next = current_block;
  }
  free(next_block);
}


//first_fit_memory request, return NULL if no memory is available
void* first_fit_req(size_t requested) {
    struct memoryList* tmp = head;
    while(tmp){
      if(!(tmp->alloc) && tmp->size >= requested){
        update(tmp, requested);
        return tmp->ptr;
      }
      tmp = tmp->next;
    }
  return NULL;
}//end of first_fit_memory



/*best_fit memory request, tries to find and return pointer to smallest
  available memory, returns NULL otherwise
*/
void* best_fit_req(size_t requested) {
  struct memoryList* min_memory = NULL; //the smallest suitable memory
  struct memoryList* tmp = head;
  while(tmp){
      if(!(tmp->alloc) && (tmp->size >= requested)){
        if(!min_memory){
          min_memory = tmp;
        }else if(tmp->size < min_memory->size){
          min_memory = tmp;
        }
      }
      tmp = tmp->next;
  }

  if(min_memory){
    update(min_memory,requested);
    return min_memory->ptr;
  }
  return NULL;
}//end of best_fit_req



/*worst_fit memory request, tries to find and return pointer to largest
  available memory, returns NULL otherwise
*/

void* worst_fit_req(size_t requested) {
  struct memoryList* max_memory = NULL;
  struct memoryList* tmp = head;
  while(tmp){
    if(!(tmp->alloc) && (tmp->size >= requested)){
      if(!max_memory){
        max_memory = tmp;
      }else if(tmp->size > max_memory->size){
        max_memory = tmp;
      }
    }
    tmp = tmp->next;
  }

  if(max_memory){
    update(max_memory,requested);
    return max_memory->ptr;
  }else{
    return NULL;
  }
}//end of worst_fit_req


/*next_fit memory request, tries to find and return pointer to available memory,
after the last allocated memory, returns NULL otherwise
*/
void* next_fit_req(size_t requested) {
  struct memoryList* tmp = next;
  int done = 0;
  while(!done){
    if(!(tmp->alloc) && tmp->size >= requested){
      update(tmp, requested);
      return tmp->ptr;
    }
    if(!(tmp = tmp->next)){
      tmp=head;
    }
    done = (tmp == next);
  }

  return NULL;
}

/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
  if(head != NULL)
    free(head);

	myMemory = malloc(sz);

	/* TODO: Initialize memory management structure. */
  head = malloc(sizeof(struct memoryList));
  head->size = sz;
  head->alloc = 0;
  head->ptr = myMemory;
  head->next = NULL;
  head->prev = NULL;
  next = head;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1
 */

void *mymalloc(size_t requested)
{
	assert((int)myStrategy > 0);
	switch (myStrategy)
	  {
	  case NotSet:
	            return NULL;
	  case First:
              return (first_fit_req(requested));
	  case Best:
              return best_fit_req(requested);
	  case Worst:
	            return worst_fit_req(requested);
	  case Next:
              return next_fit_req(requested);
	  }
	return NULL;
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block){
  struct memoryList* tmp = head;
  while(tmp){
    if(tmp->ptr == block){
      break;
    }
    tmp = tmp->next;
  }
  if(tmp){
    tmp->alloc = 0;
    if(tmp!= head && !(tmp->prev->alloc)){
      join_left(tmp);
    }else if((tmp->next) && !(tmp->next->alloc)){
      join_right(tmp);
    }
  }
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when we refer to "memory" here, we mean the
 * memory pool this module manages via initmem/mymalloc/myfree.
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes(){
  int count = 0;
  struct memoryList* tmp = head;
  while(tmp){
    count += !tmp->alloc;
    tmp = tmp->next;
  }
  return count;
}

/* Get the number of bytes allocated */
int mem_allocated() {
  return mySize - mem_free();
}

/* Number of non-allocated bytes */
int mem_free() {
  int count = 0;
  struct memoryList* tmp = head;
  while(tmp){
    if(!tmp->alloc){
      count+= tmp->size;
    }
    tmp = tmp->next;
  }
  return count;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free() {
  int largest_free = 0;
  struct memoryList* tmp = head;
  while(tmp){
    if(!(tmp->alloc) && (tmp->size > largest_free)){
      largest_free = tmp->size;
    }
    tmp = tmp->next;
  }
  return largest_free;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size) {
  int count = 0;
  struct memoryList* tmp = head;
  while(tmp){
    if(!(tmp->alloc) && (tmp->size <= size)){
      count++;
    }
    tmp = tmp->next;
  }
  return count;
}

char mem_is_alloc(void *ptr){
  if(ptr >= myMemory && ptr <= mySize){
    struct memoryList* tmp = head->next;
    while(tmp){
      if(!tmp->next){
        return tmp->alloc;
      }else{
        if(ptr < tmp) {
          return tmp->alloc;
        }
      }
      tmp = tmp->next;
      return tmp->alloc;
    }
  }
}

/*
 * Feel free to use these functions, but do not modify them.
 * The test code uses them, but you may ind them useful.
 */

//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy.
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/*
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	return;
}

/* Use this function to track memory allocation performance.
 * This function does not depend on your implementation,
 * but on the functions you wrote above.
 */
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
        strategies strat;
	void *a, *b, *c, *d, *e;
	if(argc > 1)
	  strat = strategyFromString(argv[1]);
	else
	  strat = First;


	/* A simple example.
	   Each algorithm should produce a different layout. */

	initmem(strat,500);

	a = mymalloc(100);
	b = mymalloc(100);
	c = mymalloc(100);
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);

	print_memory();
	print_memory_status();

}
