#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#define N 20

#define COL_FMT "%-6d%-11lu%-10lu%-14lu%-14lu%-18lu%-14lu%-12lu\n"

static volatile unsigned sink;

static void
burn_cpu(void) {
  for (int i = 0; i < 2000000; i++)
    sink += i * 31;
  for (int i = 0; i < 2000000; i++)
    sink += i * 17;
  for (int i = 0; i < 2000000; i++)
    sink += i * 17;
}

static void
print_row(const struct sched_stats *s, unsigned long now_tick) {
  unsigned long turnaround = now_tick > s->creation_tick
                                 ? now_tick - s->creation_tick
                                 : 0;
  unsigned long response =
      (s->first_sched_tick > s->creation_tick)
          ? s->first_sched_tick - s->creation_tick
          : 0;

  printf(COL_FMT, getpid(), turnaround, response, s->ticks_running,
         s->creation_tick, s->first_sched_tick, s->tot_rescheds,
         s->tot_wakeups);
}

static void
print_header(void) {
  printf("%-6s%-11s%-10s%-14s%-14s%-18s%-14s%-12s\n", "PID", "TURNAROUND", "RESPONSE",
         "RUNNING_TICKS", "CREATION_TICK", "FIRST_SCHED_TICK", "TOT_RESCHEDS", "TOT_WAKEUPS");
}

int
main(void) {
  printf("=== Scheduler Benchmark: forking %d children ===\n", N);
  print_header();

  for (int i = 0; i < N; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      burn_cpu();
      burn_cpu();
      burn_cpu();
      burn_cpu();
      struct sched_stats s;
      long now = sched_get_stats(&s);
      if (now >= 0) {
        print_row(&s, (unsigned long)now);
      } else
        printf("sched_get_stats failed\n");
      exit(0);
    }

    /* boost PID 5 so its larger CPU share is visible
       (lottery scheduler only: RR returns -ENOTSUP) */
    if (pid == 5) {
      long r = sched_set_prio(pid, 500);
      printf("sched_set_prio(5, 500) -> %ld\n", r);
    }
    if (pid == 10) {
        long r = sched_set_prio(pid, 1);
        printf("sched_set_prio(10, 1) -> %ld\n", r);
    }
  }

  for (int i = 0; i < N; i++) {
    wait(NULL);
  }

  return 0;
}
