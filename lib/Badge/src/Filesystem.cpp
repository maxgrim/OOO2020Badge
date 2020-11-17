#include "Filesystem.h"
#include "FlagCrypto.h"

#include <FS.h>

const char *encryptedFlag PROGMEM = "ikJ2gRfAbWoEeQ9xGNcGYTofyxGwM8fda69wIpH9B2zY23lepbSXLmdbibOggmD2C+tn2gnCyxa8ucwc65Ov0In3cEa6c27a5map0uYKi6AeMWP7+yTPWv/dPfa+d9NJLpikjQ==";

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