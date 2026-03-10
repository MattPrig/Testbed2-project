
#include "debug.h"

Debug::Debug(Stream* _stream, uint8_t _debug_level) {
  stream = _stream;
  debug_level = _debug_level;
}

void Debug::setDebugLevel(uint8_t _debug_level) {
  debug_level = _debug_level;
}

void Debug::println(String x, int debug_level) {
  //a function to print debug messages only if they are ob=ver the DEBUG_LEVEL

  if (debug_level <= DEBUG_LEVEL) {
    if (debug_level == DEBUG_WARNING) {
      stream->print("\033[33m"); // Set text color to yellow
    } else if (debug_level == DEBUG_ERROR) {
      stream->print("\033[31m"); // Set text color to red
    }
    stream->print(x);
    stream->println("\033[0m"); // Reset text color
  }
}

void Debug::print(String x, int debug_level) {
  if (debug_level <= DEBUG_LEVEL) {
        if (debug_level == DEBUG_WARNING) {
      stream->print("\033[33m"); // Set text color to yellow
    } else if (debug_level == DEBUG_ERROR) {
      stream->print("\033[31m"); // Set text color to red
    }
    stream->print(x);
    stream->print("\033[0m"); // Reset text color

  }
}
