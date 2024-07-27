#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>

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
	char * abc = 0x0;
	size_t size = 10;

	int result = gen1_entropy(&abc, size);

	if (result != 0) return result;

	assert(abc != 0x0);

	out(abc, size);

	munmap(abc, size);

	return 0;
}
