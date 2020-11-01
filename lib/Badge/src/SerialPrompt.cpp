#include "SerialPrompt.h"
#include "Debug.h"
#include "Badge.h"
#include "FlagCrypto.h"

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

static DoneCallback executionDone;

static char buffer[BUFFER_LENGTH + 1];

static uint8_t commandTableIndex = 0;
static cmd_t commandTable[TABLE_LENGTH];
static uint8_t commandCharsRead = 0;

static char *strings[ARGUMENTS_LENGTH];

static char *cmdHiddenFlagEncrypted = "\xde\xd4\x27\x30\xf5\x08\xa6\xf5\x84\x91\x1b\xf6\xc8\x66\x94\xbd\xeb\xfc\x10\x32\xec\x63\x80\xc5\x64\xfa\xc9\xe2\x50\x86\xd2\x15\x82\xa6\xad\x7c\x13";
static char *cmdHiddenFlagKey = "\x9d\x80\x61\x4b\xc0\x6c\x94\xc1\xb6\xa6\x78\xc1\xaa\x02\xf2\x85\xd9\xce\x22\x06\xd8\x01\xe2\xf7\x01\xcc\xf9\xdb\x33\xb1\xe5\x27\xe3\xc4\x98\x1f\x6e";

static void handleSerialInput();
static Task tHandleSerialInput(TASK_IMMEDIATE, TASK_FOREVER, &handleSerialInput);

static bool active = true;

void printPrompt()
{
    Serial.printf("\r\n$ ");
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
            serialPromptDeactivate();
            executionDone = badgeRequestExecution(SERIAL_PROMPT);
            commandTable[i].func(argc, strings);
        }
    }

    if (!executed)
    {
        Serial.println(F("Unknown command"));
        serialPromptActivate();
    }
}

void cmdLs(uint8_t argc, char **argv)
{
    Serial.println(F("flag.txt\tmorse\tcatchTheLed.sh\tfastCalculation.sh"));
    serialPromptActivate();
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
            morseCodeSetup(serialPromptActivate);
        }
        else if (strcmp(argv[1], "./fastCalculation.sh") == 0)
        {
            fastCalculationSetup(serialPromptActivate);
        }
        else if (strcmp(argv[1], "./catchTheLed.sh") == 0)
        {
            catchTheLedSetup(serialPromptActivate);
        }
        else
        {
            Serial.println(F("Invalid command"));
            serialPromptActivate();
        }
    }
    else
    {
        Serial.println(F("Invalid command"));
        serialPromptActivate();
    }
}

void cmdCat(uint8_t argc, char **argv)
{
    if (argc != 0 && (strcmp(argv[1], "flag.txt") == 0 || strcmp(argv[1], "./flag.txt") == 0))
    {
        Settings settings = settingsGetSettings();

        if (settings.isAdmin == 1)
        {
            const char *encryptedFlag = "PhyfKjW9yh/Meend+rdMihzLahFenHCfrqlmWL26e77ZN8Kehb6qbiEesGgj7nWp";
            const uint8_t aesKey[AES_BLOCK_SIZE] = {0xb5, 0x43, 0x86, 0x98, 0xb7, 0xa0, 0xe4, 0x9f, 0xf8, 0xbc, 0x47, 0x76, 0xc4, 0xe0, 0xb6, 0xe0};
            const uint8_t aesIV[AES_BLOCK_SIZE] = {0xe3, 0xf0, 0x29, 0xae, 0xb9, 0xbf, 0x9b, 0x4e, 0xb9, 0xad, 0x89, 0xab, 0x06, 0xde, 0x2a, 0x62};
            char destination[strlen(encryptedFlag)];

            cryptoGetFlagAES(aesKey, aesIV, encryptedFlag, destination);
            Serial.printf("%s\r\n", destination);
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

    serialPromptActivate();
}

void cmdHidden(uint8_t argc, char **argv)
{
    uint8_t size = 37;

    cryptoGetFlagXOR(cmdHiddenFlagEncrypted, size, cmdHiddenFlagKey, size);

    Serial.println(F("[...Engine goes...]\tBzzzz Bzzzzz\r\n[...AI voice says...]\tSuper Laser activated. Ready to fire.\r\n\r\n[...Badge says...]\tYou found the hidden command! Well done young reverse engineer! Here is your reward.\r\n"));
    Serial.println(cmdHiddenFlagEncrypted);

    serialPromptActivate();
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

    serialPromptActivate();
}

static void handleSerialInput()
{
    while (Serial.available())
    {
        yield();

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

void serialPromptActivate()
{
    if (executionDone != NULL)
    {
        executionDone();
        executionDone = NULL;
    }

    active = true;

    badgeTaskScheduler.addTask(tHandleSerialInput);
    tHandleSerialInput.enable();

    printPrompt();
}

void serialPromptDeactivate()
{
    active = false;

    badgeTaskScheduler.deleteTask(tHandleSerialInput);
    tHandleSerialInput.disable();
}

void serialPromptSetup()
{
    addCommand("help", "Print help", false, cmdHelp);
    addCommand("reboot", "Reboot the badge", false, cmdReboot);
    addCommand("ls", "List files", false, cmdLs);
    addCommand("sh", "Execute file", false, cmdSh);
    addCommand("cat", "Print file", false, cmdCat);

    // The hidden flag command
    addCommand("BzzBzzD3pl0yL4z3rz", "Hidden option", true, cmdHidden);

    serialPromptActivate();
}