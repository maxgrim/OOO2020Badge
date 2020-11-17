#include "RainMan.h"

#include "../Badge.h"
#include "../FlagCrypto.h"

#include <Arduino.h>

static void (*doneCallbackF)();

static void countdown();
static Task tCountdown(1000, 3, &countdown);

static void generateCalculation();
static Task tGenerateCalculation(TASK_IMMEDIATE, TASK_ONCE, &generateCalculation);

static void handleAnswer();
static Task tHandleAnswer(100, TASK_ONCE, &handleAnswer);

static int currentCorrectAnswer;

static uint8_t correctAnswersRequired = 30;
static uint8_t correctAnswersCounter = 0;

static void deactivateRainMan()
{
    badgeTaskScheduler.deleteTask(tCountdown);
    badgeTaskScheduler.deleteTask(tGenerateCalculation);
    badgeTaskScheduler.deleteTask(tHandleAnswer);
}

static void countdown()
{
    Serial.printf("%d... ", 4 - tCountdown.getRunCounter());

    if (tCountdown.isLastIteration())
    {
        Serial.printf("\r\n");
        tGenerateCalculation.restartDelayed(1000);
    }
}

static void generateCalculation()
{
    int randomOperator = (int)random((long)0, (long)6);
    int a = (int)random((long)-10000, (long)10000);
    int b = (int)random((long)-10000, (long)10000);

    switch (randomOperator)
    {
    case 0:
        Serial.printf("%ld + %ld\r\n", a, b);
#ifdef DEBUG
        Serial.printf("  = %ld\r\n", a + b);
#endif
        currentCorrectAnswer = a + b;
        break;
    case 1:
        Serial.printf("%ld - %ld\r\n", a, b);
#ifdef DEBUG
        Serial.printf("  = %ld\r\n", a - b);
#endif
        currentCorrectAnswer = a - b;
        break;
    case 2:
        Serial.printf("%ld * %ld\r\n", a, b);
#ifdef DEBUG
        Serial.printf("  = %ld\r\n", a * b);
#endif
        currentCorrectAnswer = a * b;
        break;
    case 3:
        Serial.printf("%ld %% %ld\r\n", a, b);
#ifdef DEBUG
        Serial.printf("  = %ld\r\n", a % b);
#endif
        currentCorrectAnswer = a % b;
        break;
    case 4:
        Serial.printf("%ld XOR %ld\r\n", a, b);
#ifdef DEBUG
        Serial.printf("  = %ld\r\n", a ^ b);
#endif
        currentCorrectAnswer = a ^ b;
        break;
    case 5:
        Serial.printf("%ld AND %ld\r\n", a, b);
#ifdef DEBUG
        Serial.printf("  = %ld\r\n", a & b);
#endif
        currentCorrectAnswer = a & b;
        break;
    }

    Serial.print(F("Your answer: "));

    tHandleAnswer.restartDelayed();
}

void handleAnswer()
{
    Serial.println();
    Serial.println();

    if (!Serial.available())
    {
        Serial.print(F("Too late!\r\n"));
        deactivateRainMan();
        if (doneCallbackF != NULL)
        {
            doneCallbackF();
        }

        return;
    }
    else
    {
        int answer = Serial.parseInt();
        while (Serial.available())
        {
            Serial.read();
        }

        if (currentCorrectAnswer != 0 && answer == 0)
        {
            Serial.print(F("  >> Invalid input <<\r\n\r\n"));
            correctAnswersCounter = 0;
        }
        else if (currentCorrectAnswer != answer)
        {
            Serial.print(F("  >> Incorrect! <<\r\n\r\n"));
            correctAnswersCounter = 0;
        }
        else
        {
            Serial.print(F(" >> CORRECT! <<\r\n\r\n"));
            correctAnswersCounter++;
        }
    }

    if (correctAnswersCounter == correctAnswersRequired)
    {
        char flag[38];
        cryptoGetFlag(&flag[0], sizeof(flag), 128);

        Serial.print(F("It's raining flags, hallelujah, it's raining flags: "));
        Serial.println(flag);

        deactivateRainMan();
        if (doneCallbackF != NULL)
        {
            doneCallbackF();
        }

        return;
    }
    else
    {
        tGenerateCalculation.restartDelayed();
    }
}

void rainManSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    Serial.print(F("\r\n\
\r\n███╗   ███╗ █████╗ ██╗  ██╗███████╗                  \
\r\n████╗ ████║██╔══██╗██║ ██╔╝██╔════╝                  \
\r\n██╔████╔██║███████║█████╔╝ █████╗                    \
\r\n██║╚██╔╝██║██╔══██║██╔═██╗ ██╔══╝                    \
\r\n██║ ╚═╝ ██║██║  ██║██║  ██╗███████╗                  \
\r\n╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝                  \
\r\n                                                     \
\r\n        ██╗████████╗                                 \
\r\n        ██║╚══██╔══╝                                 \
\r\n        ██║   ██║                                    \
\r\n        ██║   ██║                                    \
\r\n        ██║   ██║                                    \
\r\n        ╚═╝   ╚═╝                                    \
\r\n                                                     \
\r\n                ██████╗  █████╗ ██╗███╗   ██╗        \
\r\n                ██╔══██╗██╔══██╗██║████╗  ██║        \
\r\n                ██████╔╝███████║██║██╔██╗ ██║        \
\r\n                ██╔══██╗██╔══██║██║██║╚██╗██║        \
\r\n                ██║  ██║██║  ██║██║██║ ╚████║        \
\r\n                ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝        \
\r\n                                                     \
\r\n                        ███╗   ███╗ █████╗ ███╗   ██╗\
\r\n                        ████╗ ████║██╔══██╗████╗  ██║\
\r\n                        ██╔████╔██║███████║██╔██╗ ██║\
\r\n                        ██║╚██╔╝██║██╔══██║██║╚██╗██║\
\r\n                        ██║ ╚═╝ ██║██║  ██║██║ ╚████║\
\r\n                        ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝\
\r\n                                                     \
\r\n\r\n"));

    randomSeed(analogRead(0));

    badgeTaskScheduler.addTask(tCountdown);
    badgeTaskScheduler.addTask(tGenerateCalculation);
    badgeTaskScheduler.addTask(tHandleAnswer);

    tCountdown.restart();
}
