#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>
#include <string.h>

#include "benchmark.h"


#define NBYTES (10000000 * 16)

void out(const char *, char *, size_t);

int main() {
	const size_t size = NBYTES;

	char * gen1_buffer = 0x0;
	char * message = 0x0;
	char * ciphertext = 0x0;

	double elapset_time_ms; int result;
	TIMED_BLOCK(elapset_time_ms, {
		result = gen1_entropy(&gen1_buffer, size);
	});

	result = gen1_entropy(&message, size);
	result = gen1_entropy(&ciphertext, size);

	printf("Elapsed time: %f ms\n", elapset_time_ms);

	if (result != 0) return result;

	assert(gen1_buffer != 0x0);

	neon_encrypt(message, gen1_buffer, ciphertext, size);

	out("key.dat", gen1_buffer, size);
	out("message.dat", message, size);
	out("ciphertext.dat", ciphertext, size);

	neon_encrypt(ciphertext, gen1_buffer, message, size);
	out("message_translated.dat", message, size);

	munmap(gen1_buffer, size);

	return 0;
}

void out(const char * name, char * data, size_t size) {
	FILE *file = fopen(name, "wb"); // Open the file for writing in binary mode
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