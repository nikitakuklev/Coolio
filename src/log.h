#include <Arduino.h>
#include "constants.h"

#ifndef logger_h
#define logger_h

class Logger {
  public:
    static void println(String s, int level);
    static void println(String s);
    static void println(__FlashStringHelper* s);
    static void print(String s, int level);
    static void print(String s);
    static void print(__FlashStringHelper* s);
};

#endif