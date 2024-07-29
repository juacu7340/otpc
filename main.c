#include "otpc.h"

#include <assert.h>
#include <sys/mman.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // arc4random family

#include "benchmark.h"


#define NBYTES (1 * 16)

void out(const char *, char *, size_t);
void test_neon_stnd(char *, char *, char *, char *, size_t);

int main() {
	const size_t nbytes = NBYTES;

	void * message = 0x0;
	message = mmap(0, nbytes, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (message == (void *)(-1)) {
		return -1;
	}

	arc4random_buf(message, nbytes);

	out("message.dat", message, nbytes);

	munmap(message, nbytes);

	double elapset_time_ms; int result;

	TIMED_BLOCK(elapset_time_ms, {
		otpc_encrypt("message.dat", "key.dat", "ciphertext.dat");
	});
	printf("(neon) encrypt: %f ms\n", elapset_time_ms);

	TIMED_BLOCK(elapset_time_ms, {
		otpc_decrypt("ciphertext.dat", "key.dat", "message_translated.dat");
	});
	printf("(neon) decrypt: %f ms\n", elapset_time_ms);

	return 0;
}

	//const size_t nbytes = NBYTES;

	//char * gen1_buffer = 0x0;
	//char * message = 0x0;
	//char * message_translated = 0x0;
	//char * ciphertext = 0x0;

	//double elapset_time_ms; int result;

	//gen1_entropy(&gen1_buffer, nbytes);
	//gen1_entropy(&message, nbytes);
	//gen1_entropy(&message_translated, nbytes);
	//gen1_entropy(&ciphertext, nbytes);

	//test_neon_stnd(message, gen1_buffer, ciphertext, message_translated, nbytes);

	//out("key.dat", gen1_buffer, nbytes);
	//out("message.dat", message, nbytes);
	//out("ciphertext.dat", ciphertext, nbytes);
	//out("message_translated.dat", message_translated, nbytes);

	//munmap(gen1_buffer, nbytes);
	//munmap(message, nbytes);
	//munmap(message_translated, nbytes);
	//munmap(ciphertext, nbytes);



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

void test_neon_stnd(char * message, char * key, char * ciphertext, char * message_translated, size_t nbytes) {
	double elapset_time_ms; int result;

#if defined (__aarch64__) && defined (__ARM_NEON)  
	// Neon benchmark
	TIMED_BLOCK(elapset_time_ms, {
		neon_encrypt(message, key, ciphertext, nbytes);
	});
	printf("(neon) encrypt: %f ms\n", elapset_time_ms);


	TIMED_BLOCK(elapset_time_ms, {
		neon_decrypt(ciphertext, key, message_translated, nbytes);
	});
	printf("(neon) decrypt: %f ms\n", elapset_time_ms);
#endif

	// Standard benchmark
	TIMED_BLOCK(elapset_time_ms, {
		standard_encrypt(message, key, ciphertext, nbytes);
	});
	printf("(stnd) encrypt: %f ms\n", elapset_time_ms);


	TIMED_BLOCK(elapset_time_ms, {
		standard_decrypt(ciphertext, key, message_translated, nbytes);
	});
	printf("(stnd) decrypt: %f ms\n", elapset_time_ms);
}