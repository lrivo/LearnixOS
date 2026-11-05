#pragma once
#include <learnix/types.h>
#include <uapi/learnix/sched.h>

struct wait_queue {
  struct process *proc;
  struct wait_queue *next;
};
