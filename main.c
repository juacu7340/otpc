#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>

int main() {
	char * abc = 0x0;
	size_t size = 5;

	int result = gen1_entropy(&abc, size);

	if (result != 0) return result;

	assert(abc != 0x0);

	for (size_t idx = 0; idx < size; ++idx) {
		printf("%d\n", abc[idx]);
	}


	munmap(abc, 256);

	return 0;
}
