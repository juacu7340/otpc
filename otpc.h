#ifndef OTPC_H
#define OTPC_H
#include <stddef.h>
//

extern int gen1_entropy(char ** addr, size_t size);
extern void gen2_entropy(char ** addr, size_t size);
extern int otpc_encrypt(const char * message_path, const char * key_path, const char * ciphertext_path);
extern int otpc_decrypt(/**/);

//
#endif

