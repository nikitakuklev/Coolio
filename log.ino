// TODO - replace with preprocessor if check
static void Log::println(String s, int level) {
  if (level <= DEBUG) {
    Serial.println(s);
  }
}

static void Log::println(String s) {
  Serial.println(s);
}

static void Log::println(__FlashStringHelper* fp) {
  Serial.println(fp);
}

static void Log::print(String s, int level) {
  if (level <= DEBUG) {
    Serial.print(s);
  }
}

static void Log::print(String s) {
    Serial.print(s);
}

static void Log::print(__FlashStringHelper* fp) {
  Serial.println(fp);
}
