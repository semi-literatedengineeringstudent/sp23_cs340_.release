/**
 * Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma pack(1)  

typedef struct _metadata_entry_t {
  size_t capacity;
  unsigned char free;
  struct _metadata_entry_t *next_free;
  struct _metadata_entry_t *prev_free;
} metadata_entry_t;

typedef struct _boundary_tag {
  size_t capacity;
} boundary_tag;

static int split_criterion = 128;//if a memory block can have capacity > split criterion, we split and create a new free block
static int coalescing_criterion = 128;//if a memory block we want to coalesce have > coalescing_criterion , we combine and create a new free block and delete old block
static void *heap_bottom = NULL;
static void *heap_top = NULL;
static metadata_entry_t *free_list_head = NULL;

void print_heap_stat() {
  if (free_list_head == NULL) {
    printf("%s", "no free blocks available. \n");
    return;
  }
  printf("-- Start of Heap (%p) --\n", heap_bottom);
  metadata_entry_t *current = free_list_head;
  while(current != NULL) {
    printf("metadata for memory %p: (%p, size=%ld, isFreed=%d)\n", (void *)current + sizeof(metadata_entry_t), current, current->capacity, current->free);
    current = current->next_free;
  }
  printf("-- End of Heap (%p) --\n", heap_top);
}

void add_to_free_list(metadata_entry_t * metadata_node) {
  metadata_node->free = 1;
  if (free_list_head == NULL) {
    // if we have an empty list, just make the new block the head of free linked list.
    free_list_head = metadata_node;
  } else {
    // if have head, we make current head the next of new node, make new node prev of current head, and make new node current head.
    metadata_node->next_free = free_list_head;
    free_list_head->prev_free = metadata_node;
    free_list_head = metadata_node;
  }
  return;
}

void delete_from_free_list(metadata_entry_t * metadata_node) {
    int has_prev = 0;
    int has_next = 0;
    metadata_node->free = 0;
    if (metadata_node->prev_free != NULL) {
      has_prev = 1;
    }
    if (metadata_node->next_free != NULL) {
      has_next = 1;
    }
    // now consider 4 cases in which we delete current node from the free linked list.
    if (has_prev == 0 && has_next == 0) {
      // this means the meta node is the only node in the free linked list and it is in head.
      free_list_head = NULL;
    } else if (has_prev == 0 && has_next == 1) {
      // this means the meta node is in the head of the free list that has more than one elements.
      free_list_head = metadata_node->next_free;
      metadata_node->next_free->prev_free = NULL;
      metadata_node->next_free = NULL;
    } else if (has_prev == 1 && has_next == 0) {
      // this means the meta node is in the tail of the free list that has more than one elements.
      metadata_node->prev_free->next_free = NULL;
      metadata_node->prev_free = NULL;
      // we don't have global var for tail of the list so we are fine.
    } else {
      // this means the meta node is in between 2 free block.
      metadata_node->prev_free->next_free = metadata_node->next_free;
      metadata_node->next_free->prev_free = metadata_node->prev_free;
      metadata_node->prev_free = NULL;
      metadata_node->next_free = NULL;
    }
    return;

}

void coalesce(metadata_entry_t *prev_node, metadata_entry_t *current_node, metadata_entry_t *next_node) {
  // coalesce a node and its prev
  int coalesce_prev = 0;
  int coalesce_next = 0;
  if (prev_node != NULL && prev_node->free == 1 && prev_node->capacity > coalescing_criterion) {
    coalesce_prev = 1;
  }
  if (next_node != NULL && next_node->free == 1 && next_node->capacity > coalescing_criterion) {
    coalesce_next = 1;
  }

  if (coalesce_prev == 0 && coalesce_next == 0) {
    // if current node cannot coalesce with both prev and next node.
    add_to_free_list(current_node);
  } else if (coalesce_prev == 1 && coalesce_next == 0) {
    // can coalesce with prev node;
    delete_from_free_list(prev_node);
    prev_node->capacity = prev_node->capacity + sizeof(boundary_tag) + 
    sizeof(metadata_entry_t) + current_node->capacity;
    boundary_tag *new_Btag = (boundary_tag *)((unsigned long)prev_node + sizeof(metadata_entry_t) + prev_node->capacity);
    new_Btag->capacity = prev_node->capacity;
    add_to_free_list(prev_node);
  } else if (coalesce_prev == 0 && coalesce_next == 1) {
    delete_from_free_list(next_node);
    current_node->capacity = current_node->capacity + sizeof(boundary_tag) + 
    sizeof(metadata_entry_t) + next_node->capacity;
    boundary_tag *new_Btag = (boundary_tag *)((unsigned long)current_node + sizeof(metadata_entry_t) + current_node->capacity);
    new_Btag->capacity = current_node->capacity;
    add_to_free_list(current_node);
  } else {
    delete_from_free_list(prev_node);
    delete_from_free_list(next_node);
    prev_node->capacity = prev_node->capacity + sizeof(boundary_tag) + 
    sizeof(metadata_entry_t) + current_node->capacity + sizeof(boundary_tag) +
    sizeof(metadata_entry_t) + next_node->capacity;
    boundary_tag *new_Btag = (boundary_tag *)((unsigned long)prev_node + sizeof(metadata_entry_t) + prev_node->capacity);
    new_Btag->capacity = prev_node->capacity;
    add_to_free_list(prev_node);
  }
  return;
}

void split(metadata_entry_t * metadata_node, size_t size_to_used) {
  if (size_to_used < metadata_node->capacity && metadata_node->capacity - sizeof(metadata_entry_t) - sizeof(boundary_tag) - size_to_used > split_criterion) {
    size_t new_capacity = metadata_node->capacity - sizeof(metadata_entry_t) - sizeof(boundary_tag) - size_to_used;
    boundary_tag *metadata_node_Btag_new = (boundary_tag *)((unsigned long)metadata_node + sizeof(metadata_entry_t) + size_to_used);
    metadata_node_Btag_new->capacity = size_to_used;
    metadata_node->capacity = size_to_used;
    metadata_node->free = 0;
    delete_from_free_list(metadata_node);
    // now add the splited node into the head of linked list, give metadata node has been successfully deleted from the free list.
    metadata_entry_t *new_node = (metadata_entry_t *)((unsigned long)metadata_node + sizeof(metadata_entry_t) + size_to_used + sizeof(boundary_tag));
    new_node->capacity = new_capacity;
    new_node->free = 0;
    new_node->prev_free = NULL;
    new_node->next_free = NULL;
    boundary_tag *new_boundary_tag = (boundary_tag *)((unsigned long)new_node + sizeof(metadata_entry_t) + new_node->capacity);
    new_boundary_tag->capacity = new_capacity;
    free((void *)((unsigned long) new_node + sizeof(metadata_entry_t)));
    return;
  } else {
    metadata_node->free = 0;
    delete_from_free_list(metadata_node);
    return;
  }
}




/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
  // implement calloc:
  size_t total_size = num * size;

  void * mem_ptr = malloc(total_size);
  if (mem_ptr != NULL) {
    memset(mem_ptr, 0, total_size);
    return mem_ptr;
  } 
  return NULL;
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
  // implement malloc:
  metadata_entry_t *current = free_list_head;
  metadata_entry_t *chosen = NULL;
  if (heap_bottom == NULL) {
    heap_bottom = sbrk(0);
  }
  if(size == 0) {
    return NULL;
  }
  while(current != NULL) {
    //implement best fit to reduce internal fragmentation. 
    //using free list, O(n) should have small n.
    if (current->capacity >= size) {
      if (chosen == NULL || (chosen != NULL && current->capacity < chosen->capacity)) {
        chosen = current;
      }
    }
    current = current->next_free;
  }
  if (chosen != NULL) {
    split(chosen, size);
    return (void *)((unsigned long)chosen + sizeof(metadata_entry_t));
  }

  chosen = sbrk(sizeof(metadata_entry_t) + size + sizeof(boundary_tag));
  if(chosen == (void*)-1) {
    return NULL;
  }
  void *ptr = (void *)((unsigned long)chosen + sizeof(metadata_entry_t));
  boundary_tag *Btag = (boundary_tag*)((unsigned long)chosen + sizeof(metadata_entry_t) + size);
  
  chosen->capacity = size;
  chosen->free = 0;
  chosen->next_free = NULL;
  chosen->prev_free = NULL;

  Btag->capacity = size;

  heap_top = (void *)((unsigned long)chosen + sizeof(metadata_entry_t) + chosen->capacity + sizeof(boundary_tag));
  return ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
  // implement free
  if (ptr == NULL) {
    return;
  }
  metadata_entry_t *current_node = (metadata_entry_t *)((unsigned long)ptr - sizeof(metadata_entry_t));
  if (current_node->free == 1) {
    return;
  }
  metadata_entry_t *prev_node = NULL;
  metadata_entry_t *next_node = NULL;
  if ((void *)current_node > (void *)heap_bottom) {
    // if current node is not at lowest position of heap.
    boundary_tag *prev_Btag = (boundary_tag *)((unsigned long)current_node - sizeof(boundary_tag));
    size_t prev_capacity = prev_Btag->capacity;
    prev_node = (metadata_entry_t *)((unsigned long)current_node - sizeof(boundary_tag) - prev_capacity - sizeof(metadata_entry_t));
  }
  if ((void *)((unsigned long)current_node + sizeof(metadata_entry_t) + current_node->capacity + sizeof(boundary_tag)) < (void *)heap_top){
    next_node = (metadata_entry_t *)((unsigned long)current_node + sizeof(metadata_entry_t) + current_node->capacity + sizeof(boundary_tag));
  }
  coalesce(prev_node, current_node, next_node);
  
  return;
}


/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {

  // implement realloc:
  if (ptr == NULL) {
    void* new_ptr = malloc(size);
    if (new_ptr == NULL) {
      return NULL;
    } else {
      return new_ptr;
    }
  }
  if (size == 0) {
    free(ptr);
    return NULL;
  }
  metadata_entry_t * old_node = (metadata_entry_t *)((unsigned long)ptr - sizeof(metadata_entry_t));
  size_t old_size = old_node->capacity;

  size_t min_size = 0;
  if (old_size < size) {
    min_size = old_size;
  } else {
    min_size = size;
  }
  
  split(old_node, size);
  if (old_node->capacity == size) {
    return ptr;
  } else {
    void* new_ptr = malloc(size);
    if (new_ptr == NULL) {
      return NULL;
    }
    memcpy(new_ptr, ptr, min_size);
    free(ptr);
    return new_ptr;
  }
}
