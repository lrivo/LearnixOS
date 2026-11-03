# RDSEED

`RDSEED` is the AMD64 instruction Learnix uses to obtain seed material from the processor. The assembly wrapper returns a 64-bit value when the instruction reports success. The architecture-specific byte interface is `arch_rand_bytes()`; Learnix uses it to seed the ChaCha20 CSPRNG rather than issuing `RDSEED` for every random request.

Implementation: [`rdseed.asm`](../../kernel/arch/amd64/rdseed.asm) and [`arch_rand_bytes()` in `cpu.c`](../../kernel/arch/amd64/cpu.c). The instruction is specified in AMD's [AMD64 Architecture Programmer's Manual, Volumes 1–5](https://docs.amd.com/v/u/en-US/40332_4.11_APM_Vol1-5).

## One 64-bit draw

`rdseed_u64(uint64_t *out)` receives its output pointer in `RDI`. The assembly tries `RDSEED RAX` up to 100 times:

1. If the carry flag is set, the instruction produced a value. The wrapper stores `RAX` through `out` and returns status `0`.
2. If the carry flag is clear, the wrapper decrements its retry count. It executes `PAUSE` and retries while attempts remain.
3. After 100 unsuccessful attempts, it returns `-1` and does not write to `out`.

The instruction may report that no seed is currently available; the carry-flag check is therefore part of the interface, not an optional optimization.

## Filling a byte buffer

`arch_rand_bytes(vec, n)` repeatedly asks `rdseed_u64()` for a 64-bit word, copies up to eight bytes into the caller's buffer, and advances until it has filled `n` bytes. The final request copies only the remaining number of bytes when `n` is not a multiple of eight. The API is declared in [`rand.h`](../../kernel/include/learnix/lib/rand.h).

At boot, `rand_init()` calls `arch_rand_bytes()` to seed the ChaCha20 state; subsequent calls to `rand_bytes()` draw from the software CSPRNG. See the [security guide](../../kernel/security/README.md) for the ChaCha20 path and its consumers.

## Current limitations

There is no CPUID check before the kernel executes `RDSEED`. A processor without the instruction will fault rather than gracefully selecting another entropy source.

The assembly reports retry exhaustion with `-1`, but `arch_rand_bytes()` currently ignores that return value and returns `0` unconditionally. On an exhausted draw, the assembly does not initialize its output word; the caller still copies from that local variable. As a result, callers such as `rand_init()` cannot detect an RDSEED failure through the current byte-interface return status.
