#include "otpc.h"

#include <stdlib.h> // arc4random family
#include <stdarg.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/random.h> // getentropy family
#include <sys/errno.h>

#include <fcntl.h>
#include <assert.h>

#include <arm_neon.h>


// Warining: message_buf & key_buf must be 16 
// Size of SIMD register for ARM-M1 cpu is 128 bits
int neon_encrypt(void * message_buf, void * key_buf, void * ciphertext_buf, size_t nbytes) {
	size_t idx = 0;
	
	while (idx < nbytes) {
		int8x16_t nth_chunk128_message = vld1q_s8((int8_t *) &message_buf[idx]);
		int8x16_t nth_chunk128_key = vld1q_s8((int8_t *) &key_buf[idx]);

		int8x16_t nth_chunk128_ciphertext = veorq_s8(nth_chunk128_message, nth_chunk128_key);

		vst1q_s8((int8_t*)&ciphertext_buf[idx], nth_chunk128_ciphertext);
		idx += 16;
	}

	return 0;
}

int neon_decrypt(void * ciphertext_buf, void * key_buf, void * message_buf, size_t nbytes) {
	size_t idx = 0;
	
	while (idx < nbytes) {
		int8x16_t nth_chunk128_ciphertext = vld1q_s8((int8_t *) &ciphertext_buf[idx]);
		int8x16_t nth_chunk128_key = vld1q_s8((int8_t *) &key_buf[idx]);

		int8x16_t nth_chunk128_message = veorq_s8(nth_chunk128_ciphertext, nth_chunk128_key);

		vst1q_s8((int8_t*)&message_buf[idx], nth_chunk128_message);
		idx += 16;
	}

	return 0;
}

// Points addr to a buffer initialized with random generated data
// uses system-specific entropy to seed pseudo-number generator
int gen1_entropy(char ** addr, const size_t size) {
	char * n_addr = 0x0;

	n_addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (n_addr == (void *)(-1)) {
		return errno;
	}

	arc4random_buf(n_addr, size);

	(*addr) = n_addr;
	return 0;
}

// TODO: Benchmark this...
// We are assuming nbytes % 16 = 0 for now
int gen2_entropy(char ** addr, const size_t nbytes) {
	void * n_addr = 0x0;

	n_addr = mmap(0, nbytes, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (n_addr == (void *)(-1)) {
		return errno;
	}

	//size_t bytes = 0;
	//while (bytes < nbytes) {
	//	int8x16_t * current_chunk = &(n_addr[bytes]); 
	//	int8x16_t nth_chunk128_addr = vld1q_s8(current_chunk);

	//	getentropy(&nth_chunk128_addr, sizeof(int8x16_t));
	//	int8_t * foo = (int8_t *)(&nth_chunk128_addr);

	//	foo[0x0] = foo[0x0] % 26;
	//	foo[0x1] = foo[0x1] % 26;
	//	foo[0x2] = foo[0x2] % 26;
	//	foo[0x3] = foo[0x3] % 26;
	//	foo[0x4] = foo[0x4] % 26;
	//	foo[0x5] = foo[0x5] % 26;
	//	foo[0x6] = foo[0x6] % 26;
	//	foo[0x7] = foo[0x7] % 26;
	//	foo[0x8] = foo[0x8] % 26;
	//	foo[0x9] = foo[0x9] % 26;
	//	foo[0xA] = foo[0xA] % 26;
	//	foo[0xB] = foo[0xB] % 26;
	//	foo[0xC] = foo[0xC] % 26;
	//	foo[0xD] = foo[0xD] % 26;
	//	foo[0xE] = foo[0xE] % 26;
	//	foo[0xF] = foo[0xF] % 26;

	//	vst1q_s8(current_chunk, nth_chunk128_addr);

	//	bytes += sizeof(int8x16_t); // 16 bytes since a SIMD register fits v128 (bits)
	//}

	(*addr) = n_addr;
	return 0;
}

int otpc_encrypt(const char * message_path, const char * key_path, const char * ciphertext_path,
	int (*entropy_fn)(char **, const size_t)) {

	//if (check_null(4, message_path, key_path, ciphertext_path, entropy_fn) == 0) {
	//	return -1;
	//}

	// TODO: measure performance difference between fopen() built-in buffered IO
	// versus open() non-buffered IO

	mode_t proc_mask = umask(0066); // Might be too restrictive...

	mode_t r_mode = S_IRUSR; // Read permissions 
	mode_t w_mode = r_mode | S_IWUSR;

	int message_fd = open(message_path, O_RDONLY | O_CREAT, r_mode);

	if (message_fd == -1) {
		// Error (check errno)
		goto encrypt_return;
	}

	int key_fd = open(key_path, O_WRONLY | O_CREAT, w_mode);

	if (key_fd == -1) {
		// Error (check errno)
		goto encrypt_cleanup_message_fd;
	}

	int ciphertext_fd = open(ciphertext_path, O_WRONLY | O_CREAT, w_mode);

	if (ciphertext_fd == -1) {
		// Error (check errno)
		goto encrypt_cleanup_message_fd;
	}


	struct stat message_stat;
	if (fstat(message_fd, &message_stat) == -1) {
		// Error (check errno)
		goto encrypt_cleanup_message_fd;
	}

	size_t message_size = message_stat.st_size; // Input file size, in bytes
	size_t key_size, ciphertext_size; // Output file sizes, in bytes
	 
	// Output file sizes are the same as input file size, in bytes
	key_size = ciphertext_size = message_size;

	char * message_mmap = mmap(0, message_size, PROT_READ, MAP_PRIVATE, message_fd, 0);
	
	if (message_mmap == (void *)(-1)) {
		// Error (check errno)
		goto encrypt_cleanup;
	}

encrypt_cleanup:

encrypt_cleanup_mmap:
	munmap(message_mmap, message_size);

encrypt_cleanup_fd:

encrypt_cleanup_ciphertext_fd:
	close(ciphertext_fd);
encrypt_cleanup_key_fd:
	close(key_fd);
encrypt_cleanup_message_fd:
	close(message_fd);

	// Restore original umask mode
	umask(proc_mask);
encrypt_return:
	return 0;
}

int otpc_decrypt(/**/) {
	return 0;
}

// Checks if the given arguments are not null
// Returns 0 if at least one argument is null, 1 otherwise
int check_null(size_t argc, ...) {
	va_list args;
	va_start(args, argc);

	int result = 1;
	int idx = 0;

	while (idx < argc) {
		void * ptr = va_arg(args, void *);
		if (ptr == 0) {
			result = 0;
			break;
		}
		idx = idx + 1;
	}

	va_end(args);
	return result;
}