#ifndef CRYPTO_H_SEEN
#define CRYPTO_H_SEEN

#include <stddef.h>
#include <stdint.h>

void cryptoGetFlag(char *destination, size_t destination_length, int input);
void cryptoGetFlagXOR(char *buffer, uint8_t buffer_size, char *key, uint8_t key_size);

#endif
