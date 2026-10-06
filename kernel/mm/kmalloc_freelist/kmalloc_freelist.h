#include <learnix/types.h>

/* structures definitions */
struct chunk
{
  #define CNK_ALLOC (1 << 0U)
  uint32_t flags;
  uint32_t size;
  struct chunk *next;
};

struct heap
{
  struct chunk *wilderness;
  struct chunk *freelist;

  uint32_t nchunks;
  uint32_t nalloc;
};

/* a chunk must have at least a pointer-width of data
 * space for storing the *next field. */
#define ALLOC_MIN sizeof(uintptr_t)
#define IS_CNK_ALLOC(c) (((struct chunk*)(c))->flags & CNK_ALLOC)
#define CNK_NXT(c) ((struct chunk*)((char*)(c) + 8 + (c)->size))
#define CNK_HDR(c) ((struct chunk*)((char*)(c) - 8))
