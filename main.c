#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>
#include <string.h>

#include "benchmark.h"


#define NBYTES (10000000 * 16)

void out(const char *, char *, size_t);

	//double elapset_time_ms; int result;
	//TIMED_BLOCK(elapset_time_ms, {
	//	result = gen1_entropy(&gen1_buffer, size);
	//});
	//printf("Elapsed time: %f ms\n", elapset_time_ms);

int main() {
	const size_t size = NBYTES;

	char * gen1_buffer = 0x0;
	char * message = 0x0;
	char * message_translated = 0x0;
	char * ciphertext = 0x0;

	double elapset_time_ms; int result;

	gen1_entropy(&gen1_buffer, size);
	gen1_entropy(&message, size);
	gen1_entropy(&message_translated, size);
	gen1_entropy(&ciphertext, size);

#if defined (__aarch64__) && defined (__ARM_NEON)  
		// Neon benchmark
		TIMED_BLOCK(elapset_time_ms, {
			neon_encrypt(message, gen1_buffer, ciphertext, size);
		});
		printf("(neon) encrypt: %f ms\n", elapset_time_ms);


		TIMED_BLOCK(elapset_time_ms, {
			neon_decrypt(ciphertext, gen1_buffer, message_translated, size);
		});
		printf("(neon) decrypt: %f ms\n", elapset_time_ms);
#endif

		// Standard benchmark
		TIMED_BLOCK(elapset_time_ms, {
			standard_encrypt(message, gen1_buffer, ciphertext, size);
		});
		printf("(stnd) encrypt: %f ms\n", elapset_time_ms);


		TIMED_BLOCK(elapset_time_ms, {
			standard_decrypt(ciphertext, gen1_buffer, message_translated, size);
		});
		printf("(stnd) decrypt: %f ms\n", elapset_time_ms);

	out("key.dat", gen1_buffer, size);
	out("message.dat", message, size);
	out("ciphertext.dat", ciphertext, size);
	out("message_translated.dat", message_translated, size);

	munmap(gen1_buffer, size);
	munmap(message, size);
	munmap(message_translated, size);
	munmap(ciphertext, size);

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