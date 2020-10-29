#ifndef RGB_H_SEEN
#define RGB_H_SEEN

#include <stdint.h>
#include <TaskSchedulerDeclarations.h>

void rgbSetup();

void rgbSetSingleLed(int led, uint32_t color);
void rgbBlinkSingleLed(int led, int times, uint32_t color, TaskOnDisable doneCallback);
void rgbSetAllLeds(uint32_t color);

#endif