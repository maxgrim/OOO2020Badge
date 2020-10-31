#ifndef RGB_H_SEEN
#define RGB_H_SEEN

#include <stdint.h>
#include <TaskSchedulerDeclarations.h>

void rgbSetSingleLed(uint8_t nr, uint32_t color);
void rgbBlinkSingleLed(uint8_t nr, uint16_t times, uint32_t color, TaskOnDisable doneCallback);
void rgbSetAllLeds(uint32_t color);
void rgbSetBrightness(uint8_t brightness);
void rgbClear();
void rgbShow();
void rgbSetup();

#endif