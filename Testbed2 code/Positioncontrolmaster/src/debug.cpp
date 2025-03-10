
#include "debug.h"

void debug_println(String x, int debug_level) {
  //a function to print debug messages only if they are ob=ver the DEBUG_LEVEL

  if (debug_level <= DEBUG_LEVEL) {
    if (debug_level == DEBUG_WARNING) {
      Serial.print("\033[33m"); // Set text color to yellow
    } else if (debug_level == DEBUG_ERROR) {
      Serial.print("\033[31m"); // Set text color to red
    }
    Serial.print(x);
    Serial.println("\033[0m"); // Reset text color
  }
}
void debug_print(String x, int debug_level) {
  if (debug_level <= DEBUG_LEVEL) {
        if (debug_level == DEBUG_WARNING) {
      Serial.print("\033[33m"); // Set text color to yellow
    } else if (debug_level == DEBUG_ERROR) {
      Serial.print("\033[31m"); // Set text color to red
    }
    Serial.print(x);
    Serial.print("\033[0m"); // Reset text color

  }
}