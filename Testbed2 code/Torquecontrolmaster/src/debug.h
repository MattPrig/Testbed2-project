#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

#define DEBUG_VERBOSE 4
#define DEBUG_NORMAL 3
#define DEBUG_WARNING 2
#define DEBUG_ERROR 1
#define DEBUG_NONE 0

#define DEBUG_LEVEL DEBUG_NORMAL

void debug_println(String x, int debug_level);

void debug_print(String x, int debug_level);

#endif