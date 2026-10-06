# Lottery Scheduler
This is a **proportional-share scheduler**, that tries to guarantee that each process gets a certain percentage of CPU time based on importance.
The importance is represented using **tickets**, and the more a process is important, the more tickets he'll get.
Then, at every timer interrupt, we hold a **lottery**, aka we randomly select a winner.

## Splitmax64 PRNG
The lottery implementation uses an internal PRNG called splitmax64 instead of relying on the ChaCha20 generator. This way we only seed the lottery
once at boot and then run on this faster generator. If we where using the ChaCha20 one we would add unnecessary latency, as reseeding happens sometimes
and a lottery doesn't need CSPRNG properties at all.

## References
- https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf
