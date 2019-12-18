

#include <assert.h> 
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;
struct _block *nextstart=NULL ;
struct _block *tempNext=NULL;
int best=9999999;
int worst=0;
struct _block *final=NULL;
struct _block *finalworst=NULL;


/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
	printf("\nheap management statistics\n");
	printf("mallocs:\t%d\n", num_mallocs );
	printf("frees:\t\t%d\n", num_frees );
	printf("reuses:\t\t%d\n", num_reuses );
	printf("grows:\t\t%d\n", num_grows );
	printf("splits:\t\t%d\n", num_splits );
	printf("coalesces:\t%d\n", num_coalesces );
	printf("blocks:\t\t%d\n", num_blocks );
	printf("requested:\t%d\n", num_requested );
	printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
	 size_t  size;         /* Size of the allocated _block of memory in bytes */
	 struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
	 struct _block *next;  /* Pointer to the next _block of allcated memory   */
	 bool   free;          /* Is this _block free?                     */
	 char   padding[3];
};


struct _block *freeList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
w */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
	 struct _block *curr = freeList;

#if defined FIT && FIT == 0
	 /* First fit */
	 while (curr && !(curr->free && curr->size >= size)) 
	 {
			*last = curr;
			curr  = curr->next;
	 }
#endif

/* Best fit picks the block that has minimum space remaining after fitting the requested size*/
#if defined BEST && BEST == 0
while(curr)
{ 
if(curr->free)
{
 if((curr->size-size) < best) /*Looking for the block with minimum difference iteratively*/
{
 final = curr; 
best =(curr->size)-size; /*updating difference*/
}

}
curr=curr->next;
}
curr=final;/* Returns the block with minimum difference*/

#endif


/*Worst fit picks the block that has maximum space remaining after fitting the requested size*/
#if defined WORST && WORST == 0
while (curr)
{
if(curr->free)
{
if((curr->size-size)>worst) /*looking for the maximum difference*/
{
worst=(curr->size-size);/*uodating difference*/
finalworst=curr; 
}

}
curr=curr->next;
}
curr=finalworst;/* block with maximum difference is returned*/

#endif

#if defined NEXT && NEXT == 0


/* It performs first fit and when called next time it starts off where it left the first time*/
if(nextstart) 
{
curr=nextstart; 
}
while (curr && !(curr->free && curr->size >= size)) /*If the block is NULL or not suitable for first fit we move onto the next block */
	 {
			*last = curr;
			curr  = curr->next;
	 }
nextstart=curr; /*To save the block that was returned after first fit*/

#endif

	 return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
	 /* Request more space from OS */
	 struct _block *curr = (struct _block *)sbrk(0);
	 struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

	 assert(curr == prev);
	 

	 /* OS allocation failed */
	 if (curr == (struct _block *)-1) 
	 {
			return NULL;
	 }

	 /* Update freeList if not set */
	 if (freeList == NULL) 
	 {
			freeList = curr;
	 }

	 /* Attach new _block to prev _block */
	 if (last) 
	 {
			last->next = curr;
	 }
					 
			 num_blocks++; /* Everytime heap grows the number of free blocks increases*/


	 /* Update _block metadata */
	 curr->size = size;
	 curr->next = NULL;
	 curr->free = false;
	 max_heap+=size; /* The maximum size to which the heap ever grows */
	 return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
	
			 
	 if( atexit_registered == 0 )
	 {
			atexit_registered = 1;
			atexit( printStatistics );
	 }

	 /* Align to multiple of 4 */
	 size = ALIGN4(size);
	 num_requested=num_requested+size;
	 /* Handle 0 size */
	 if (size == 0) 
	 {
			return NULL;
	 }

	 /* Look for free _block */
	 struct _block *last = freeList;
	 struct _block *next = findFreeBlock(&last, size);
 
/* If the block available is greater than the size requested we split the remaining space into a new block*/
/* TO: split if possible */


if(next && next->size >size)
{
tempNext=next->next; /* A temporary block is used to save the next block */
 next->next = (void*)next+size+sizeof(struct _block); /* The remaining space is now the new next block*/
next->next->size = sizeof(struct _block) + next->size - size ;
next->next->free=true;
if(tempNext==NULL) 
{
next->next->next=NULL;
}
else
{
next->next->next=tempNext;/* if the old block is not null it is now the next block for the new block*/
}
num_splits++;
num_blocks++; /* the number of free blocks increases everytime we split*/
}

	
/*    Could not find free _block, so grow heap */
	 if (next == NULL) 
	 {
				 
	 next = growHeap(last, size);
	 num_grows++; /*everytime a new block is requested*/
	
	 }
else
{

num_reuses++; /* if it finds a free block that can be reused*/
}

	 /* Could not find free _block or grow heap, so just return NULL */
	 if (next == NULL) 
	 {
			return NULL;
	 }
	else
{

num_mallocs++; /* for every successful malloc*/
} 
	 /* Mark _block as in use */
	 next->free = false;

	 /* Return data address associated with _block */

	 return BLOCK_DATA(next);
	 
}

/* Calloc calls malloc and sets data to 0 */

void *calloc(size_t nmemb, size_t size)
{
void *ptr=malloc(nmemb * size);
memset(ptr,0,nmemb * size);
return ptr;
}

/* Realloc calls malloc and transfers the data from the old block to new block*/

void *realloc(void *ptr, size_t size)
{ 
int oldsize=BLOCK_HEADER(ptr)->size;
void *ptr1=malloc(size);
memcpy(ptr1,ptr,oldsize);
free(ptr);
return ptr1;
}


/* * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
	 if (ptr == NULL) 
	 {
			return;
	 }

	 /* Make _block as free */
	 struct _block *curr = BLOCK_HEADER(ptr);
	 assert(curr->free == 0);
	 curr->free = true;
	 num_frees++;

/* If two adjacent blocks are free we combine them into one*/
	 /* TODO: Coalesce free _blocks if needed */


if(curr->next!=NULL && curr->free && curr->next->free)
{
 curr->size = curr->size + curr->next->size;
if(curr->next->next!=NULL){
curr->next = curr->next->next;
}
else{
 curr->next=NULL;
}
 num_coalesces++;
 num_blocks--; /*the count of free blocks decreases*/
}

}
 /*vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
