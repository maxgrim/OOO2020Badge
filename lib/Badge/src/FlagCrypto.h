#ifndef CRYPTO_H_SEEN
#define CRYPTO_H_SEEN

#include <stddef.h>
#include <stdint.h>

#define AES_BLOCK_SIZE 16

void cryptoAESBufferSize(const char *text, int &length);
void cryptoGetFlag(char *destination, size_t destination_length, int input);
void cryptoGetFlagXOR(char *buffer, uint8_t buffer_size, char *key, uint8_t key_size);
void cryptoGetFlagAES(const uint8_t *aesKey, const uint8_t *aesIV, const char *encryptedFlag, char *destination);

#endif
