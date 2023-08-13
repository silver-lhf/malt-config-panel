#include <Arduino.h>

#define CONFIG_SIZE (10240)
#define CONFIG_FILENAME ("/config.json")

String readJson(const char *path = CONFIG_FILENAME);
void saveJson(const char *content, const char *path = CONFIG_FILENAME);
