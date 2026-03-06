#include <lib/string.h>
#include <stdint.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
  asm volatile("rep movsb" : "+D"(dest), "+S"(src), "+c"(n)::"memory");
  return dest;
}

/*
 * https://linuxvox.com/blog/64-bit-linux-performance-issue-with-memset/#32-bit-i386-memset
 */
void *memset(void *s, int c, size_t n) {
  asm volatile("rep stosb" ::"D"(s), "a"(c), "c"(n) : "cc", "memory");
  return s;
}

void *memmove(void *dest, const void *src, size_t n) {
  uint8_t *pdest = (uint8_t *)dest;
  const uint8_t *psrc = (const uint8_t *)src;

  if (src > dest) {
    for (size_t i = 0; i < n; i++) {
      pdest[i] = psrc[i];
    }
  } else if (src < dest) {
    for (size_t i = n; i > 0; i--) {
      pdest[i - 1] = psrc[i - 1];
    }
  }

  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = (const uint8_t *)s1;
  const uint8_t *p2 = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] < p2[i] ? -1 : 1;
    }
  }

  return 0;
}
