#ifndef BADGEDEBUG_H_SEEN
#define BADGEDEBUG_H_SEEN

// enable additional debug output
#ifdef BADGE_DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) do { Serial.printf(x, __VA_ARGS__); } while (0)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

#endif