#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>

#include <time.h>

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

	double elapset_time_ms;
	int result;
	clock_t start_time, end_time, elapsed_time;

	// gen2
	start_time = clock();

	result = gen2_entropy(&gen2_buffer, size);

	end_time = clock();
	elapsed_time = end_time - start_time;

	elapset_time_ms = (double)elapsed_time * 1000.0 / CLOCKS_PER_SEC;

	printf("Elapsed time: %f ms\n", elapset_time_ms);

	if (result != 0) return result;
	assert(gen2_buffer != 0x0);
	out(gen2_buffer, size);
	munmap(gen2_buffer, size);

	// gen1
	start_time = clock();

	result = gen1_entropy(&gen1_buffer, size);

	end_time = clock();
	elapsed_time = end_time - start_time;

	elapset_time_ms = (double)elapsed_time * 1000.0 / CLOCKS_PER_SEC;

	printf("Elapsed time: %f ms\n", elapset_time_ms);

	if (result != 0) return result;
	assert(gen1_buffer != 0x0);
	out(gen1_buffer, size);
	munmap(gen1_buffer, size);


	return 0;
}
