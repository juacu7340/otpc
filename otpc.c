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

		// Debu:
		nth_chunk128_message = vsubq_s8(nth_chunk128_message, vdupq_n_s8(0x41));
		
		int8x16_t nth_chunk128_ciphertext = vaddq_s8(nth_chunk128_message, nth_chunk128_key);

		int8_t * m_ptr = (int8_t *)(&nth_chunk128_ciphertext);

		m_ptr[0x0] = m_ptr[0x0] % 26;
		m_ptr[0x1] = m_ptr[0x1] % 26;
		m_ptr[0x2] = m_ptr[0x2] % 26;
		m_ptr[0x3] = m_ptr[0x3] % 26;
		m_ptr[0x4] = m_ptr[0x4] % 26;
		m_ptr[0x5] = m_ptr[0x5] % 26;
		m_ptr[0x6] = m_ptr[0x6] % 26;
		m_ptr[0x7] = m_ptr[0x7] % 26;
		m_ptr[0x8] = m_ptr[0x8] % 26;
		m_ptr[0x9] = m_ptr[0x9] % 26;
		m_ptr[0xA] = m_ptr[0xA] % 26;
		m_ptr[0xB] = m_ptr[0xB] % 26;
		m_ptr[0xC] = m_ptr[0xC] % 26;
		m_ptr[0xD] = m_ptr[0xD] % 26;
		m_ptr[0xE] = m_ptr[0xE] % 26;
		m_ptr[0xF] = m_ptr[0xF] % 26;

		vst1q_s8((int8_t*)&ciphertext_buf[idx], nth_chunk128_ciphertext);
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
int gen2_entropy(char ** addr, const size_t size) {
	char * n_addr = 0x0;
	assert(size <= 256);

	n_addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (n_addr == (void *)(-1)) {
		return errno;
	}
	getentropy(n_addr, size); // 256 size limit (read docs)

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