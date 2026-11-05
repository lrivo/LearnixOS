#pragma once
#include <sys/types.h>
#include <learnix/pmm.h>

int pmm_stats_start(void);
int pmm_stats_get(struct pmm_stats *stats);
