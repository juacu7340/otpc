#ifndef OTPC_H
#define OTPC_H
#include <stddef.h>
//

extern int gen1_entropy(char ** addr, const size_t size);
extern int gen2_entropy(char ** addr, const size_t size);
extern int otpc_encrypt(const char * message_path, const char * key_path, const char * ciphertext_path,
    int (*entropy)(char **, const size_t));
extern int otpc_decrypt(/**/);

//
#endif

