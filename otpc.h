#ifndef OTPC_H
#define OTPC_H
#include <stddef.h>
//

extern int otpc_encrypt(const char *, const char *, const char *);
extern int otpc_decrypt(const char *, const char *, const char *);

extern int neon_encrypt(void *, void *, void *, size_t);
extern int neon_decrypt(void *, void *, void *, size_t);

int standard_encrypt(void *, void *, void *, size_t);
int standard_decrypt(void *, void *, void *, size_t);
//
#endif

