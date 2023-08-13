#include <LittleFS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "fileHelper.h"
#include <ESPAsyncWebServer.h>

WiFiManager wm;
AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request)
{
    Serial.println("NOT FOUND");
    request->send(404, "text/plain", "Not found");
}

void setup()
{
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_AP_STA);
    Serial.begin(115200);
    WiFiManagerParameter config("config", "config json", readJson().c_str(), CONFIG_SIZE);
    wm.addParameter(&config);
    wm.setSaveParamsCallback([&config]()
                             { saveJson(config.getValue()); });
    // set custom ip for portal
    Serial.print("Auto Connect ... ");
    bool res = wm.autoConnect(); // auto generated AP name from chipid
                                 // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
                                 // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    // if(!res) {
    //     Serial.println("Failed to connect");
    //     ESP.restart();
    // }
    // else {
    //     //if you get here you have connected to the WiFi
    //     Serial.println("connected...yeey :)");
    // }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
                  response->addHeader("Content-Encoding", "gzip");
                  request->send(response);
                  //  request->send(LittleFS, "/index.html", "text/html");
              });

    server.on("/storage", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                    File root = LittleFS.open("/storage");
                    File file = root.openNextFile();
                    String ret;
                    while (file)
                    {
                        ret += file.name();
                        ret += ",";
                        file = root.openNextFile();       
                    }
                    // request->send(200, "application/json", ret); 
                    request->send(200, "text/plain", ret); });

    server.on(
        "/api/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
                        String apiJson;
                        for (size_t i = 0; i < len; i++) {
                            apiJson += char(data[i]);
                        }
                        saveJson(apiJson.c_str(), "/api.json");
                        Serial.println(apiJson);
                        request->send(200); });
    server.on(
        "/api/load", HTTP_GET, [](AsyncWebServerRequest *request)
        {
                        String apiJson = readJson("/api.json").c_str();
                        request->send(200, "text/plain", apiJson); });

    server.onNotFound(notFound);

    server.begin();
}

void loop()
{
}
