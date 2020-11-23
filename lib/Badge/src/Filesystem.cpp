#include "Filesystem.h"
#include "FlagCrypto.h"

#include <FS.h>

const char *encryptedFlag = "\x8a\x42\x76\x81\x17\xc0\x6d\x6a\x04\x79\x0f\x71\x18\xd7\x06\x61\x3a\x1f\xcb\x11\xb0\x33\xc7\xdd\x6b\xaf\x70\x22\x91\xfd\x07\x6c\xd8\xdb\x79\x5e\xa5\xb4\x97\x2e\x67\x5b\x89\xb3\xa0\x82\x60\xf6\x0b\xeb\x67\xda\x09\xc2\xcb\x16\xbc\xb9\xcc\x1c\xeb\x93\xaf\xd0\x89\xf7\x70\x46\xba\x73\x6e\xda\xe6\x66\xa9\xd2\xe6\x0a\x8b\xa0\x1e\x31\x63\xfb\xfb\x24\xcf\x5a\xff\xdd\x3d\xf6\xbe\x77\xd3\x49\x2e\x98\xa4\x8d";

void filesystemCatFlag()
{
    File f = SPIFFS.open(F("/coordinates.out"), "r");

    if (!f)
    {
        Serial.println(F("Trying to write to '/coordinates.out' but the file does not exist."));
    }
    else
    {
        if (f.size() == 0)
        {
            f = SPIFFS.open("/coordinates.out", "w");
            const uint8_t aesKey[AES_BLOCK_SIZE] = {0xc4, 0xd4, 0x9c, 0x48, 0x07, 0x87, 0x76, 0xc1, 0xf9, 0xd9, 0x83, 0xd2, 0xd4, 0xf7, 0x6c, 0x54};
            const uint8_t aesIV[AES_BLOCK_SIZE] = {0x09, 0x9f, 0x66, 0x8b, 0x6c, 0x25, 0xec, 0x84, 0xfe, 0xbe, 0x59, 0xc3, 0x62, 0xa1, 0xd7, 0xcb};
            char destination[strlen(encryptedFlag)];

            cryptoGetFlagAES(aesKey, aesIV, encryptedFlag, destination);
            f.write(destination);

            Serial.println(F("Wrote the coordinates to '/coordinates.out'"));
        }
        else
        {
            Serial.println(F("The '/coordinates.out' file already has content, not overwriting with coordinates."));
        }
    }

    f.close();
}

void filesystemSetup()
{
    if (!SPIFFS.begin())
    {
        Serial.println(F("[x] Failed to initialize SpaceshipPortableInterForceFortifiedSystem filesystem!\r\n"));
    }
}