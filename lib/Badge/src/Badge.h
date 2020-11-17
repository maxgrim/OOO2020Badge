#ifndef BADGE_H_SEEN
#define BADGE_H_SEEN

#include <TaskSchedulerDeclarations.h>
#include <WString.h>

extern Scheduler badgeTaskScheduler;

typedef enum {
    MENU,
    SERIAL_PROMPT
} ExecutionMethod;

typedef void (*DoneCallback)();

bool badgeSetup(size_t eepromSize, size_t eepromStartAddress, bool forceStart);
void badgeLoop();
DoneCallback badgeRequestExecution(ExecutionMethod method);
void badgeFatalError(const __FlashStringHelper *message);

#endif