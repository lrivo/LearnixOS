#include <learnix/types.h>
#include <learnix/lib/kprintf.h>
#include <learnix/lib/kpanic.h>
#include <learnix/lib/rand.h>
#include <learnix/lib/string.h>

#define CHACHA_CONST "expand 32-byte k"

#define QR(x, a, b, c, d) \
  x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 16); \
  x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 12); \
  x[a] += x[b]; x[d] = rotl32(x[d] ^ x[a], 8); \
  x[c] += x[d]; x[b] = rotl32(x[b] ^ x[c], 7);

struct chacha20_ctx
{
  // initial state [CHACHA_CONST, key, counter, nonce]
  uint32_t state[16];
  // 512-bit output
  uint32_t keystream[16];
  // how many bytes we've read from keystream
  size_t cursor;
};

/* Global state of the CSPRNG. */
static struct chacha20_ctx ctx;

/* Performs a left rotation of x by n bytes. */
static inline uint32_t
rotl32(uint32_t x, int n)
{
  return (x << n) | ( x >> (32 - n));
}

/* Generates the next ChaCha20 block into ctx.keystream. */
static void
chacha20_blk_nxt()
{
  // copy initial state
  memcpy(ctx.keystream, ctx.state, 64);


  // 10 double-rounds
  for (int i = 0; i < 10; i++)
  {
    QR(ctx.keystream, 0, 4, 8, 12)
    QR(ctx.keystream, 1, 5, 9, 13)
    QR(ctx.keystream, 2, 6, 10, 14)
    QR(ctx.keystream, 3, 7, 11, 15)
    QR(ctx.keystream, 0, 5, 10, 15)
    QR(ctx.keystream, 1, 6, 11, 12)
    QR(ctx.keystream, 2, 7, 8, 13)
    QR(ctx.keystream, 3, 4, 9, 14)
  }

  // feed-forward
  for (int i = 0; i < 16; i++)
    ctx.keystream[i] += ctx.state[i];
}


void
rand_init()
{
  // seed ChaCha20 using hardware TRNG
  memcpy(ctx.state, CHACHA_CONST, 4 * sizeof(uint32_t));
  kassert(arch_rand_bytes(&ctx.state[4], 8 * sizeof(uint32_t)) >= 0);
  kassert(arch_rand_bytes(&ctx.state[13], 3 * sizeof(uint32_t)) >= 0);
  ctx.state[12] = 0;

  // generate the first keystrem
  chacha20_blk_nxt();
  ctx.cursor = 0;
}

// TODO: can refactor this with a memcpy()
void
rand_bytes(void *vec, size_t n)
{
  uint8_t *v = (uint8_t*)vec, *k = (uint8_t*)ctx.keystream;
  for (size_t i = 0; i < n; i++)
  {
    if (ctx.cursor >= 64)
    {
      ctx.state[12]++;
      ctx.cursor = 0;
      chacha20_blk_nxt();
    }
    v[i] = k[ctx.cursor++];
  }
}
