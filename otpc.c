#include "otpc.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h> // arc4random family
#include <sys/random.h> // getentropy family

#include <sys/errno.h>

#include <stdarg.h>

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


// Points addr to a buffer initialized with random generated data
// uses system-specific entropy to seed pseudo-number generator
int gen1_entropy(char ** addr, size_t size) {
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
void gen2_entropy(char ** addr, size_t size) {
	char * n_addr = 0x0;

	n_addr = mmap(0, size, PROT_READ, MAP_ANON, -1, 0);
	getentropy(n_addr, size);

	(*addr) = n_addr;
}

int otpc_encrypt(const char * message_path, const char * key_path, const char * ciphertext_path) {
	if (check_null(message_path, key_path, ciphertext_path) == 0) {
		return -1;
	}

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
