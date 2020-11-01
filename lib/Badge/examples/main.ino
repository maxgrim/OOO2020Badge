#include <Badge.h>

#define EEPSIZE 4096

void setup()
{
    badgeSetup(EEPSIZE, 2900, true);
}

void loop()
{
    badgeLoop();
}
