#include "Crypto.h"
#include <Arduino.h>
#include <stdio.h>

#define S_SWAP(a, b)  \
    do                \
    {                 \
        int t = S[a]; \
        S[a] = S[b];  \
        S[b] = t;     \
    } while (0)

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