#include <learnix/mm/pmm_stats.h>

volatile int pmm_stats_active = 0;

/* Per-operation sample buffer. */
struct pmm_op_samples
{
  uint64_t samples[PMM_STATS_MAX];
  uint64_t count;
  uint64_t sum;
  uint64_t min;
  uint64_t max;
};

static struct pmm_op_samples alloc_samples;
static struct pmm_op_samples unref_samples;

static void
samples_record (struct pmm_op_samples *s, uint64_t cycles)
{
  /* The buffer is full: stop recording, count never lies about what the
   * statistics cover. */
  if (s->count == PMM_STATS_MAX)
    return;

  if (s->count == 0 || cycles < s->min)
    s->min = cycles;
  if (s->count == 0 || cycles > s->max)
    s->max = cycles;

  s->sum += cycles;
  s->samples[s->count++] = cycles;
}

static void
samples_sort (struct pmm_op_samples *s)
{
  for (uint64_t i = 1; i < s->count; i++)
  {
    const uint64_t v = s->samples[i];
    uint64_t j = i;
    while (j > 0 && s->samples[j - 1] > v)
    {
      s->samples[j] = s->samples[j - 1];
      j--;
    }
    s->samples[j] = v;
  }
}

static void
samples_finish (struct pmm_op_samples *s, struct pmm_op_stats *out)
{
  samples_sort (s);

  out->count = s->count;
  out->min = s->min;
  out->mean = s->count ? s->sum / s->count : 0;
  out->median = s->count ? s->samples[s->count / 2] : 0;
  out->max = s->max;
}

void
pmm_stats_start (void)
{
  alloc_samples.count = 0;
  alloc_samples.sum = 0;
  alloc_samples.min = 0;
  alloc_samples.max = 0;
  unref_samples.count = 0;
  unref_samples.sum = 0;
  unref_samples.min = 0;
  unref_samples.max = 0;

  pmm_stats_active = 1;
}

void
pmm_stats_stop (struct pmm_stats *out)
{
  pmm_stats_active = 0;

  samples_finish (&alloc_samples, &out->alloc);
  samples_finish (&unref_samples, &out->unref);
}

void
pmm_stats_record_alloc (uint64_t cycles)
{
  samples_record (&alloc_samples, cycles);
}

void
pmm_stats_record_unref (uint64_t cycles)
{
  samples_record (&unref_samples, cycles);
}
