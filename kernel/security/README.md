# security/ - Canaries and the CSPRNG

Kernel security is defense in depth (also see `todo.md`'s security section:
KASLR, KPTI, userspace ASLR). This folder holds two independent building blocks
- the pieces that talk to hardware are thin; the cryptography is built by hand.

## Stack canaries (`security/canary.c`)

A **stack canary** is a random sentinel value planted in a buffer/stack region
before untrusted data is written. If anything overwrites it, the value changes,
and the kernel can detect the overflow *before* an attacker can pivot it into a
return-address overwrite - the same idea production compilers weave into
frame layouts.

```c
void canary_generate(vaddr_t at) {
  uint64_t canary = 0;
  rand_bytes(&canary, 7);       // 7 random bytes, MSB stays 0
  *((uint64_t*)at) = canary;
}
```

Each process that uses them gets a random 56-bit canary. The design leaves the
top byte zero so a canary is never confused with a canonical pointer. For the userspace
programs, the canary is checked before a return crosses a boundary.

## ChaCha20 CSPRNG (`security/chacha20.c`)

A **CSPRNG** (Cryptographically Secure PRNG) is a random-number generator
whose output an adversary cannot predict even having seen past output. Learnix
implements one from scratch - based on the [ChaCha20](https://en.wikipedia.org/wiki/ChaCha20)
stream cipher - instead of shipping a call-from-userspace.

- **Seeding** (`rand_init`): grabs true entropy from the **hardware RNG**
  (`arch_rand_bytes`, which on x86-64 uses `rdseed`) and uses it to fill the
  ChaCha20 state's key and nonce.
- **Keystream** (`chacha20_blk_nxt`): 10 ChaCha *double rounds* (the classic
  "quarter-round" structure sweeping the 16 x 32-bit lane matrix with the
  constant `"expand 32-byte k"`), then feed-forward adds the initial state to
  produce a 512-bit block.
- **Output** (`rand_bytes`): gives bytes out one at a time, re-keying the
  counter and generating a fresh block whenever the 64-byte cursor is
  exhausted.

ChaCha20 was chosen because it is very fast in software, and it is the same
primitive behind the modern Linux CSPRNG and many TLS stacks. The
CSPRNG backs the canaries above (and, downstream, userspace ASLR).

> Why this matters: many "random" numbers in a kernel must be **unpredictable
> to an attacker** - a stack canary that can be guessed is no canary at all;
> an ASLR base that can be leaked defeats ASLR. ChaCha20 gives Learnix an
> in-house guarantee instead of trusting the environment.

Wiring: `drivers`/`arch` code seeds `rand_init()` early in `kmain` (see the
boot README); `canary.c` and future security features draw on it through
`<learnix/lib/rand.h>`.