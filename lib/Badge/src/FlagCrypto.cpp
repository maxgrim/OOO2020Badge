#include "FlagCrypto.h"
#include <Arduino.h>
#include <stdio.h>

#include <Crypto.h>
#include <base64.hpp>

#define S_SWAP(a, b)  \
    do                \
    {                 \
        int t = S[a]; \
        S[a] = S[b];  \
        S[b] = t;     \
    } while (0)

static void aesEncrypt(const uint8_t *aesKey, const uint8_t *aesIV, const char *plain_text, char *output, int length)
{
    byte enciphered[length];
    AES aesEncryptor(aesKey, aesIV, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
    aesEncryptor.process((uint8_t *)plain_text, enciphered, length);
    int encrypted_size = sizeof(enciphered);
    char encoded[encrypted_size];
    encode_base64(enciphered, encrypted_size, (unsigned char *)encoded);
    strcpy(output, encoded);
}

static void aesDecrypt(const uint8_t *aesKey, const uint8_t *aesIV, const char *enciphered, char *output, int length)
{
    length = length + 1;
    char decoded[length];
    decode_base64((unsigned char *)enciphered, (unsigned char *)decoded);
    cryptoAESBufferSize(enciphered, length);
    byte deciphered[length];
    AES aesDecryptor(aesKey, aesIV, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
    aesDecryptor.process((uint8_t *)decoded, deciphered, length);
    strcpy(output, (char *)deciphered);
}

void cryptoAESBufferSize(const char *text, int &length)
{
    int i = strlen(text);
    int buf = round(i / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    length = (buf <= i) ? buf + AES_BLOCK_SIZE : length = buf;
}

void cryptoGetFlag(char *destination, size_t length, int input)
{
    unsigned char S[256];

    int i, j = 0;

    for (i = 0; i < 256; i++)
    {
        S[i] = i;
    }

    for (i = 0; i < 256; i++)
    {
        j = (j + S[i] + input) % 256;
        S_SWAP(S[i], S[j]);
        input *= 17;
        input = input % 256;
    }

    snprintf(destination, length, "CTF{%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x}",
             S[0], S[1], S[2], S[3], S[4], S[5], S[6], S[7], S[8], S[9], S[10], S[11], S[12], S[13], S[14], S[15]);
}

void cryptoGetFlagXOR(char *buffer, uint8_t buffer_size, const char *key, uint8_t key_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        buffer[i] = buffer[i] ^ key[i % key_size];
    }
}

void cryptoGetFlagAES(const uint8_t *aesKey, const uint8_t *aesIV, const char *encryptedFlag, char *destination)
{
    aesDecrypt(aesKey, aesIV, encryptedFlag, destination, strlen(encryptedFlag));
}