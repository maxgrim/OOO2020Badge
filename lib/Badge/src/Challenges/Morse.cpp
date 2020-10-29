
#include "Morse.h"
#include "../Badge.h"
#include "../RGB.h"
#include "../Debug.h"
#include "../Crypto.h"

#include <Arduino.h>

static const struct
{
    const char letter, *code;
} MorseMap[] =
    {
        {'A', ".-"},
        {'B', "-..."},
        {'C', "-.-."},
        {'D', "-.."},
        {'E', "."},
        {'F', "..-."},
        {'G', "--."},
        {'H', "...."},
        {'I', ".."},
        {'J', ".---"},
        {'K', "-.-"},
        {'L', ".-.."},
        {'M', "--"},
        {'N', "-."},
        {'O', "---"},
        {'P', ".--."},
        {'Q', "--.-"},
        {'R', ".-."},
        {'S', "..."},
        {'T', "-"},
        {'U', "..-"},
        {'V', "...-"},
        {'W', ".--"},
        {'X', "-..-"},
        {'Y', "-.--"},
        {'Z', "--.."},
        {' ', " "}, // Gap between word, seven units

        {'1', ".----"},
        {'2', "..---"},
        {'3', "...--"},
        {'4', "....-"},
        {'5', "....."},
        {'6', "-...."},
        {'7', "--..."},
        {'8', "---.."},
        {'9', "----."},
        {'0', "-----"},
        // For the CTF we make () equal to {} for flag
        {'(', "-.--."},
        {')', "-.--.-"},
        {'{', "-.--."},
        {'}', "-.--.-"},
        // We don't need the characters listed below
        // {'"', ".-..-."},
        // {'@', ".--.-."},
        // {'&', ".-..."},
        // {'.', "·–·–·–"},
        // {',', "--..--"},
        // {'?', "..--.."},
        // {'!', "-.-.--"},
        // {':', "---..."},
        // {';', "-.-.-."},
};

void displayMorse();
void handleOff();

static Task tDisplayMorse(TASK_IMMEDIATE, TASK_FOREVER, &displayMorse);
static Task tHandleOff(TASK_IMMEDIATE, TASK_ONCE, &handleOff);

#define UNIT_LENGTH 200

unsigned int encodedMorseIndex = 0;
String encodedMorse;

static void (*doneCallbackF)();

String encodeMorse(const char *string)
{
    String encodedMorse = "";

    for (unsigned int i = 0; string[i]; ++i)
    {
        for (unsigned int j = 0; j < sizeof(MorseMap) / sizeof(*MorseMap); ++j)
        {
            if (toupper(string[i]) == MorseMap[j].letter)
            {
                encodedMorse += MorseMap[j].code;
                break;
            }
        }

        encodedMorse += " "; // Add tailing space to separate the characters
    }

    return encodedMorse;
}

void handleOff()
{
    rgbSetAllLeds(0);
}

void displayMorse()
{
    if (encodedMorseIndex <= encodedMorse.length())
    {
        switch (encodedMorse[encodedMorseIndex++])
        {
        case '.':
            rgbSetAllLeds(0xFFFF00);

            tHandleOff.restartDelayed(UNIT_LENGTH);
            tDisplayMorse.delay(UNIT_LENGTH * 2);
            break;
        case '-':
            rgbSetAllLeds(0xFFFF00);

            tHandleOff.restartDelayed(UNIT_LENGTH * 3);
            tDisplayMorse.delay(UNIT_LENGTH * 3 * 2);
            break;
        case ' ':
            tDisplayMorse.delay(UNIT_LENGTH);
            break;
        }
    }
    else
    {
        encodedMorseIndex = 0;
        badgeTaskScheduler.deleteTask(tDisplayMorse);

        if (doneCallbackF != NULL)
        {
            doneCallbackF();
        }
    }
}

void morseCodeSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;
    
    encodedMorse = encodeMorse("HELP i am not a satellite PGS(484s2397p4rrq90426s3n6o115r3670n)");
    badgeTaskScheduler.addTask(tDisplayMorse);
    badgeTaskScheduler.addTask(tHandleOff);
    tDisplayMorse.restart();
}