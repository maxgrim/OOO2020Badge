#include <Badge.h>

#define EEPSIZE 4096

void setup()
{
    badgeSetup(EEPSIZE, 3200, true);
}

void loop()
{
    badgeLoop();
}
