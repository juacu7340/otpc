#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <time.h>
#define TIMED_BLOCK(elapsed_time_ms, code_block) \
    do {\
	    clock_t start_time, end_time, elapsed_time;\
        start_time = clock();\
        code_block\
        end_time = clock();\
		elapsed_time = end_time - start_time;\
		elapset_time_ms = (double)elapsed_time * 1000.0 / CLOCKS_PER_SEC;\
    } while (0)

extern void to_csv(const char * csv_path, const char * key, double value);
#endif