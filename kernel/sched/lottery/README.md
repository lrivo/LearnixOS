# Lottery Scheduler

This is a **proportional-share scheduler**, that tries to guarantee that each process gets a certain percentage of CPU time based on importance.

The importance is represented using **tickets**, and the more a process is important, the more tickets he'll get.

Then, at every timer interrupt, we hold a **lottery**, aka we randomly select a winner.

## References
- https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched-lottery.pdf