#ifndef BADGE_H_SEEN
#define BADGE_H_SEEN

#include <TaskSchedulerDeclarations.h>

extern Scheduler badgeTaskScheduler;

bool badgeSetup(size_t eepromSize, size_t eepromStartAddress, bool forceStart);
void badgeLoop();
void badgeFatalError(const char *message);

#endif