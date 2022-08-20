#include <helpers.hpp>
#include <time.h>

int cycles_per_us __aligned(CACHE_LINE_SIZE);
uint64_t start_tsc;

static inline void cpu_serialize(void)
{
        asm volatile("xorl %%eax, %%eax\n\t"
		     "cpuid" : : : "%rax", "%rbx", "%rcx", "%rdx");
}

static void time_calibrate_tsc(void)
{
	/* TODO: New Intel CPUs report this value in CPUID */
	struct timespec sleeptime;
	sleeptime.tv_nsec = 5E8;
	sleeptime.tv_sec = 0;
	struct timespec t_start, t_end;

	cpu_serialize();
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &t_start) == 0) {
		uint64_t ns, end, start;
		double secs;

		start = rdtsc();
		nanosleep(&sleeptime, NULL);
		clock_gettime(CLOCK_MONOTONIC_RAW, &t_end);
		end = rdtscp(NULL);
		ns = ((t_end.tv_sec - t_start.tv_sec) * 1E9);
		ns += (t_end.tv_nsec - t_start.tv_nsec);

		secs = (double)ns / 1000;
		cycles_per_us = (uint64_t)((end - start) / secs);
		printf("time: detected %d ticks / us", cycles_per_us);

		/* record the start time of the binary */
		start_tsc = rdtsc();
	}

}


/**
 * time_init - global time initialization
 *
 * Returns 0 if successful, otherwise fail.
 */
void __attribute__((constructor))  time_init(void)
{
	time_calibrate_tsc();
}
