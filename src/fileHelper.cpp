#include <LittleFS.h>
#include "fileHelper.h"

String readJson(const char *path)
{
  File file = LittleFS.open(path, "r");
  if (!file)
  {
    // return empty json
    return "{}";
  }

  String json;
  for (int i = 0; i < CONFIG_SIZE; ++i)
  {
    if (!file.available())
    {
      break;
    }
    json += (char)file.read();
  }
  file.close();
  return json;
}

void saveJson(const char *content, const char *path)
{
  File file = LittleFS.open(path, "w+");
  file.print(content);
  file.close();
}
