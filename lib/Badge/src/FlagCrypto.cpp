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

#define AES_BLOCK_SIZE 16

const uint8_t aesKey[AES_BLOCK_SIZE] = {0xa3, 0x18, 0xe9, 0x26, 0x9a, 0x14, 0xdd, 0x91, 0x63, 0xe8, 0x25, 0x51, 0x75, 0x1b, 0x8f, 0x0b};
const uint8_t aesIV[AES_BLOCK_SIZE] = {0x58, 0x12, 0xa9, 0x24, 0x46, 0xdc, 0x64, 0x76, 0x51, 0x42, 0x46, 0x1c, 0xf8, 0x1d, 0x54, 0x41};

void aesBufferSize(char *text, int &length)
{
    int i = strlen(text);
    int buf = round(i / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    length = (buf <= i) ? buf + AES_BLOCK_SIZE : length = buf;
}

void aesEncrypt(char *plain_text, char *output, int length)
{
    byte enciphered[length];
    // RNG::fill(iv, AES_BLOCK_SIZE);
    AES aesEncryptor(aesKey, aesIV, AES::AES_MODE_128, AES::CIPHER_ENCRYPT);
    aesEncryptor.process((uint8_t *)plain_text, enciphered, length);
    int encrypted_size = sizeof(enciphered);
    char encoded[encrypted_size];
    encode_base64(enciphered, encrypted_size, (unsigned char *)encoded);
    strcpy(output, encoded);
}

void aesDecrypt(char *enciphered, char *output, int length)
{
    length = length + 1; //re-adjust
    char decoded[length];
    decode_base64((unsigned char *)enciphered, (unsigned char *)decoded);
    aesBufferSize(enciphered, length);
    byte deciphered[length];
    AES aesDecryptor(aesKey, aesIV, AES::AES_MODE_128, AES::CIPHER_DECRYPT);
    aesDecryptor.process((uint8_t *)decoded, deciphered, length);
    strcpy(output, (char *)deciphered);
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

void cryptoGetFlagXOR(char *buffer, uint8_t buffer_size, char *key, uint8_t key_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        buffer[i] = buffer[i] ^ key[i % key_size];
    }
}

void cryptoGetFlagAES(char *encryptedFlag, char *destination)
{
    aesDecrypt(encryptedFlag, destination, strlen(encryptedFlag));
}