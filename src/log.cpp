#include "log.h"

// TODO - replace with preprocessor if check
void Logger::println(String s, int level) {
  if (level <= DEBUG) {
    Serial.println(s);
  }
}

void Logger::println(String s) {
  Serial.println(s);
}

void Logger::println(__FlashStringHelper* fp) {
  Serial.println(fp);
}

void Logger::print(String s, int level) {
  if (level <= DEBUG) {
    Serial.print(s);
  }
}

void Logger::print(String s) {
    Serial.print(s);
}

void Logger::print(__FlashStringHelper* fp) {
  Serial.println(fp);
}
