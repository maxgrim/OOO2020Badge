#include "SerialPrompt.h"
#include "Debug.h"
#include "Badge.h"
#include "FlagCrypto.h"
#include "Menu.h"

#include "Settings.h"

// External programs to execute
#include "Challenges/Morse.h"
#include "Challenges/CombinationLock.h"
#include "Challenges/FastCalculation.h"
#include "Challenges/CatchTheLed.h"
// ----------------------------

#include <Arduino.h>

#define BUFFER_LENGTH 100
#define TABLE_LENGTH 10
#define ARGUMENTS_LENGTH 10

typedef struct _cmd_t
{
    char *cmd;
    char *description;
    bool hidden;
    void (*func)(uint8_t argc, char **argv);
} cmd_t;

char buffer[BUFFER_LENGTH + 1];

uint8_t commandTableIndex = 0;
static cmd_t commandTable[TABLE_LENGTH];
uint8_t commandCharsRead = 0;

char *strings[ARGUMENTS_LENGTH];

bool active = true;

char *cmdHiddenFlagEncrypted = "\xde\xd4\x27\x30\xf5\x08\xa6\xf5\x84\x91\x1b\xf6\xc8\x66\x94\xbd\xeb\xfc\x10\x32\xec\x63\x80\xc5\x64\xfa\xc9\xe2\x50\x86\xd2\x15\x82\xa6\xad\x7c\x13";
char *cmdHiddenFlagKey = "\x9d\x80\x61\x4b\xc0\x6c\x94\xc1\xb6\xa6\x78\xc1\xaa\x02\xf2\x85\xd9\xce\x22\x06\xd8\x01\xe2\xf7\x01\xcc\xf9\xdb\x33\xb1\xe5\x27\xe3\xc4\x98\x1f\x6e";

void printPrompt()
{
    Serial.printf("$ ");
}

void activatePrompt()
{
    active = true;
    printPrompt();
}

void deactivatePrompt()
{
    active = false;
}

void addCommand(char *name, char *description, bool hidden, void (*func)(uint8_t argc, char **argv))
{
    if (commandTableIndex >= TABLE_LENGTH)
    {
        badgeFatalError("SerialPrompt command commandTable is full");
    }

    cmd_t cmd = {};
    cmd.cmd = name;
    cmd.description = description;
    cmd.hidden = hidden;
    cmd.func = func;

    commandTable[commandTableIndex++] = cmd;
}

bool read()
{
    while (Serial.available())
    {
        char c = Serial.read();

        switch (c)
        {
        case '\r':
        case '\n':
            buffer[commandCharsRead] = '\0';
            if (commandCharsRead > 0)
            {
                // Print the newline
                Serial.println();

                commandCharsRead = 0;

                // Signal that the reading is done
                return true;
            }
        case '\b':
            if (commandCharsRead > 0)
            {
                buffer[--commandCharsRead] = '\0';

                // Erase the character from the terminal
                Serial.write('\b');
                Serial.write(' ');
                Serial.write('\b');
            }
            break;
        default:
            Serial.write(c);

            if (commandCharsRead < BUFFER_LENGTH)
            {
                buffer[commandCharsRead++] = c;
            }

            buffer[commandCharsRead] = '\0';

            break;
        }
    }

    return false;
}

void parseAndExecute()
{
    bool executed = false;
    uint8_t argc = 0;
    char *bufferPtr = strtok(buffer, " ");

    while (bufferPtr != NULL && argc < ARGUMENTS_LENGTH)
    {
        strings[argc++] = bufferPtr;
        bufferPtr = strtok(NULL, " ");
    }

    for (uint8_t i = 0; i < commandTableIndex; i++)
    {
        if (!strcmp(strings[0], commandTable[i].cmd))
        {
            executed = true;
            deactivatePrompt();
            commandTable[i].func(argc, strings);
        }
    }

    if (!executed)
    {
        Serial.println(F("Unknown command"));
        activatePrompt();
    }
}

void cmdLs(uint8_t argc, char **argv)
{
    Serial.println(F("flag.txt\tmorse\tcatchTheLed.sh\tfastCalculation.sh"));
    activatePrompt();
}

void cmdReboot(uint8_t argc, char **argv)
{
    ESP.restart();
}

void cmdSh(uint8_t argc, char **argv)
{
    if (argc != 0)
    {
        if (strcmp(argv[1], "./morse") == 0)
        {
            morseCodeSetup(activatePrompt);
        }
        else if (strcmp(argv[1], "./fastCalculation.sh") == 0)
        {
            fastCalculationSetup(activatePrompt);
        }
        else if (strcmp(argv[1], "./catchTheLed.sh") == 0)
        {
            catchTheLedSetup(activatePrompt);
        }
        else
        {
            Serial.println(F("Invalid command"));
            activatePrompt();
        }
    }
    else
    {
        Serial.println(F("Invalid command"));
        activatePrompt();
    }
}

void cmdCat(uint8_t argc, char **argv)
{
    if (argc != 0 && strcmp(argv[1], "flag.txt") == 0)
    {
        Settings settings = settingsGetSettings();

        if (settings.isAdmin == 1)
        {
            // TODO: Replace placeholder with the vigenere cipher
            Serial.printf("Here is your flag: %s\r\n", "PLACEHOLDER");
        }
        else
        {
            Serial.printf("cat: flag.txt: Permission denied\r\n");
        }
    }
    else
    {
        Serial.println(F("File not found\r\n"));
    }

    activatePrompt();
}

void cmdHidden(uint8_t argc, char **argv)
{
    uint8_t size = 37;

    cryptoGetFlagXOR(cmdHiddenFlagEncrypted, size, cmdHiddenFlagKey, size);

    Serial.println(F("[...Engine goes...]\tBzzzz Bzzzzz\r\n[...AI voice says...]\tSuper Laser activated. Ready to fire.\r\n\r\n[...Badge says...]\tYou found the hidden menu! Well done young reverse engineer! Here is your reward.\r\n"));
    Serial.println(cmdHiddenFlagEncrypted);

    activatePrompt();
}

void cmdHelp(uint8_t argc, char **argv)
{
    Serial.println(F("Badge shell, version 1.0.0(1)-release (xtensa)\r\n"));

    for (uint8_t i = 0; i < commandTableIndex; i++)
    {
        if (!commandTable[i].hidden)
        {
            Serial.printf(" %s: %s\r\n", commandTable[i].cmd, commandTable[i].description);
        }
    }

    activatePrompt();
}

void serialPromptSetup()
{
    activatePrompt();
    addCommand("help", "Print help", false, cmdHelp);
    addCommand("reboot", "Reboot the badge", false, cmdReboot);
    addCommand("ls", "List files", false, cmdLs);
    addCommand("sh", "Execute file", false, cmdSh);
    addCommand("cat", "Print file", false, cmdCat);

    // The hidden flag command
    addCommand("BzzBzzD3pl0yL4z3rz", "Hidden option", true, cmdHidden);
}

void serialPromptLoop()
{
    while (Serial.available())
    {
        // TODO: Maybe integrate active into read, read the serial buffer but do not do anything with it
        if (read())
        {
            if (active)
            {
                parseAndExecute();
            }
        }
    }
}