#pragma once
#include <sys/types.h>
#include <learnix/sched.h>

int sched_get_stats(struct sched_stats *stats);
int sched_set_prio(pid_t pid, unsigned int prio);
