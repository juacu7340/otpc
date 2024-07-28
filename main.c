#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>

#include "benchmark.h"

#include <sys/random.h> // getentropy family
#include <stdlib.h> // arc4random family
#include <sys/errno.h>

void out(char * data, size_t size) {
	FILE *file = fopen("dump.dat", "wb"); // Open the file for writing in binary mode
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    size_t written = fwrite(data, sizeof(char), size, file); // Write the buffer to the file
    if (written != size) {
        perror("Error writing to file");
    }

    fclose(file); // Close the file
}

int main() {
	size_t size = 10000;

	char * gen1_buffer = 0x0;
	char * gen2_buffer = 0x0;
	char * gen3_buffer = 0x0;

	double elapset_time_ms;
	int result;

	TIMED_BLOCK(elapset_time_ms, {
		result = gen1_entropy(&gen1_buffer, size);
	});

	printf("Elapsed time: %f ms\n", elapset_time_ms);

	if (result != 0) return result;

	assert(gen1_buffer != 0x0);

	out(gen1_buffer, size);
	munmap(gen1_buffer, size);

	return 0;
}
