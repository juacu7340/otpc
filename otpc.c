#include "otpc.h"

#include <stdio.h>

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

int standard_encrypt(void * message_buf, void * key_buf, void * ciphertext_buf, size_t nbytes) {
	size_t idx = 0;
	
	while (idx < nbytes) {
		((char *)ciphertext_buf)[idx] = ((char *)message_buf)[idx] ^ ((char *)key_buf)[idx];
		idx += 1;
	}

	return 0;
}

int standard_decrypt(void * ciphertext_buf, void * key_buf, void * message_buf, size_t nbytes) {
	size_t idx = 0;
	
	while (idx < nbytes) {
		((char *)message_buf)[idx] = ((char *)ciphertext_buf)[idx] ^ ((char *)key_buf)[idx];
		idx += 1;
	}

	return 0;
}

#define IFRET(exp, uv)\
    do {\
		if (exp == uv) {\
			return errno;\
		}\
    } while (0)

int otpc_encrypt(const char * in_message_path, const char * out_key_path, const char * out_ciphertext_path) {
	int message_fd = open(in_message_path, O_RDONLY, 0);
	IFRET(message_fd, -1);

	int key_fd = open(out_key_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	IFRET(key_fd, -1);

	int ciphertext_fd = open(out_ciphertext_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	IFRET(ciphertext_fd, -1);

	struct stat message_fstat;
	IFRET(fstat(message_fd, &message_fstat), -1);

	size_t nbytes = message_fstat.st_size;

	IFRET(ftruncate(key_fd, nbytes),-1);
	IFRET(ftruncate(ciphertext_fd, nbytes),-1);

	void * message = mmap(0, nbytes, PROT_READ, MAP_PRIVATE, message_fd, 0);
	IFRET(message, (void *)(-1));

	void * key = mmap(0, nbytes, PROT_WRITE | PROT_READ, MAP_SHARED, key_fd, 0);
	IFRET(key, (void *)(-1));

	void * ciphertext = mmap(0, nbytes, PROT_WRITE | PROT_READ, MAP_SHARED, ciphertext_fd, 0);
	IFRET(ciphertext, (void *)(-1));

	arc4random_buf(key, nbytes);
	neon_encrypt(message, key, ciphertext, nbytes);
	//standard_encrypt(message, key, ciphertext, nbytes);

	msync(ciphertext, nbytes, MS_SYNC);
	msync(key, nbytes, MS_SYNC);

	munmap(ciphertext, nbytes);
	munmap(key, nbytes);
	munmap(message, nbytes);

	close(message_fd);
	close(key_fd);
	close(ciphertext_fd);

	return 0;
}

int otpc_decrypt(const char * in_ciphertext_path, const char * in_key_path, const char * out_message_path) {
	int ciphertext_fd = open(in_ciphertext_path, O_RDONLY, 0);
	IFRET(ciphertext_fd, -1);

	int key_fd = open(in_key_path, O_RDONLY, 0);
	IFRET(key_fd, -1);

	int message_fd = open(out_message_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	IFRET(message_fd, -1);

	struct stat ciphertext_fstat;
	IFRET(fstat(ciphertext_fd, &ciphertext_fstat), -1);

	size_t nbytes = ciphertext_fstat.st_size;

	IFRET(ftruncate(message_fd, nbytes),-1);

	void * ciphertext = mmap(0, nbytes, PROT_READ, MAP_PRIVATE, ciphertext_fd, 0);
	IFRET(ciphertext, (void *)(-1));

	void * key = mmap(0, nbytes, PROT_READ, MAP_PRIVATE, key_fd, 0);
	IFRET(key, (void *)(-1));

	void * message = mmap(0, nbytes, PROT_WRITE, MAP_SHARED, message_fd, 0);
	IFRET(message, (void *)(-1));

	neon_decrypt(ciphertext, key, message, nbytes);
	//standard_decrypt(ciphertext, key, message, nbytes);

	msync(message, nbytes, MS_SYNC);

	munmap(ciphertext, nbytes);
	munmap(key, nbytes);
	munmap(message, nbytes);

	close(ciphertext_fd);
	close(key_fd);
	close(message_fd);

	return 0;
}
