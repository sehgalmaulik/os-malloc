#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

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

int count_blocks(void );

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
   num_blocks = count_blocks();
   num_mallocs--; // +1 ctr prolly due 2 init
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
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                            */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3ByaW5nIDIwMjM            */
};

// global ptr for next fit - added __attribute__((unused)) as it is used inside the #ifdef block
// and compiler was giving warnings about unused variable
struct _block *last_allocation __attribute__((unused)) = NULL;
struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 */

struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;


   // first fit
   #if defined FIT && FIT == 0
      /* First fit */
      //
      // While we haven't run off the end of the linked list and
      // while the current node we point to isn't free or isn't big enough
      // then continue to iterate over the list.  This loop ends either
      // with curr pointing to NULL, meaning we've run to the end of the list
      // without finding a node or it ends pointing to a free node that has enough
      // space for the request.
      // 
      while (curr && !(curr->free && curr->size >= size)) 
      {
         *last = curr;
         curr  = curr->next;
      }

      // if (curr != NULL)
      // {
      //    num_reuses++;
      // }
   #endif

   // best fit
   #if defined BEST && BEST == 0

      size_t smallestSize = SIZE_MAX;
      struct _block *bestFit = NULL;

      while (curr)
      {
         if (curr->free && curr->size >= size)
         {
            size_t currentDiff = curr->size - size;
            if (currentDiff < smallestSize)
            {
               smallestSize = currentDiff;
               bestFit = curr;
            }
         }
         *last = curr;
         curr = curr->next;
      }
      curr = bestFit;

      // if (curr != NULL)
      // {
      //    num_reuses++;
      // }
      
   #endif

   // worst fit
   #if defined WORST && WORST == 0


      size_t largestSize = 0;
      struct _block *worstFit = NULL;

      while(curr)
      {
         if (curr->free && curr->size >= size)
         {
            size_t currentDiff = curr->size - size;
            if (currentDiff > largestSize)
            {
               largestSize = currentDiff;
               worstFit = curr;
            }
         }
         *last = curr;
         curr = curr->next;
      }
      curr = worstFit;
   #endif

   // next fit
#if defined NEXT && NEXT == 0
/* Next fit */

// last allocation only for NEXT FIT 
// diff from last as last is tail of blocks

   if (last_allocation == NULL)
   {
      last_allocation = heapList;
   }

   // Start at the last allocation and search until the end of the list
   curr = last_allocation;

   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr = curr->next;
   }
   
   if (!curr)
   {
      return NULL; // to grow the heap
   }

   last_allocation = curr;
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

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;

   if(heapList ==NULL)
   {
      num_grows--;
   }

   num_grows++;
   max_heap += 
   size + sizeof(struct _block);
   
   return curr;
}

void split_block(struct _block *next, size_t size)
{
  if (next != NULL && next->size > (size + sizeof(struct _block) + 4))
   {
      struct _block *new_partition = NULL;
      char *current_block_start = (char *)BLOCK_DATA(next);
      char *new_block_start = current_block_start + size;
      new_partition = BLOCK_HEADER(new_block_start);

      new_partition->size = next->size - size - sizeof(struct _block);
      new_partition->next = next->next;
      new_partition->free = true;

      next->size = size;
      next->next = new_partition;

      // if(heapList ==NULL)
      // {
      //    num_splits--;
      // }
      num_splits++;
      if (last_allocation == next)
      {
         last_allocation = new_partition;
      }
   }
   else
   {
      last_allocation = next;
   }
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

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */

   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO[done]: If the block found by findFreeBlock is larger than we need then:
            If the leftover space in the new block is greater than the sizeof(_block)+4 then
            split the block.
            If the leftover space in the new block is less than the sizeof(_block)+4 then
            don't split the block.
   */

   bool found_block = (next !=NULL);
   
   

   split_block(next, size); // made a function as we use this same code in realloc 


   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }

   if (heapList == NULL)
   {
      num_reuses--; // to avoid the first block messing up ctr numbers
   }

   //if existing block found
   if(found_block)
   {
      num_reuses++;
   }
   
   /* Mark _block as in use */
   next->free = false;

   
   num_mallocs++;

   num_requested += size;

   // if(heapList == NULL)
   // {
   //    num_mallocs--;
   // }

   /* Return data address associated with _block to the user */
   return BLOCK_DATA(next);
}

/*
 * \brief free
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

   /* TODO[done]: Coalesce free _blocks.  If the next block or previous block 
            are free then combine them with this block being freed.
   */

   bool flag = false;

   struct _block *prev_checker = heapList;


   if (curr->next != NULL && curr->next->free == true)
   {
      curr->size += curr->next->size + sizeof(struct _block);
      curr->next = curr->next->next;
      flag = true;
   }

   // starting from heaplist, find the block before curr
   while (prev_checker && prev_checker->next != curr)
   {
      prev_checker = prev_checker->next;
   }

   //if block is marked as free, combine it with curr
   if (prev_checker && prev_checker->free)
   {
      prev_checker->size += curr->size + sizeof(struct _block);
      prev_checker->next = curr->next;
      flag = true;
   }

   num_frees++;
   if (flag)
   {
      num_coalesces++;
   }

}

void *calloc( size_t nmemb, size_t size )
{
   size_t total_size = nmemb * size;
   void *ptr = malloc(total_size);

   if (ptr == NULL)
   {
      return NULL;
   }

   memset(ptr, 0, total_size);

   num_requested += total_size;

   return ptr;
}

void *realloc( void *ptr, size_t size )
{

   if (ptr == NULL)
   {
      return malloc(size);
      // num_mallocs++;
   }

   if (size == 0)
   {
      free(ptr);
      return NULL;
   }

   size = ALIGN4(size);

   struct _block *curr = BLOCK_HEADER(ptr);

   if (curr->size >= size)
   {
      split_block(curr, size);
      return ptr;
   }

   else 
   {
      struct _block *realloc_block = malloc(size);
      if (realloc_block == NULL)
      {
         return NULL;
      }

      memcpy(realloc_block, ptr, curr->size);
      free(ptr);
      num_frees++;
      return realloc_block;
   }
}

int count_blocks(void )
{
   int ctr =0;
   struct _block *curr = heapList;

   while (curr != NULL )
   {
      ctr++;  
      curr = curr->next;
   }
   return ctr;
}

