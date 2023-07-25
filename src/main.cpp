#include <LittleFS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "fileHelper.h"

WiFiManager wm;

void setup() {
    LittleFS.begin(true);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_AP_STA);
    Serial.begin(115200);
    WiFiManagerParameter config("config", "config json", readJson().c_str(), CONFIG_SIZE);
    wm.addParameter(&config);
    wm.setSaveParamsCallback([&config]() {
        saveJson(config.getValue());
    });
    bool res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
 
    // if(!res) {
    //     Serial.println("Failed to connect");
    //     ESP.restart();
    // } 
    // else {
    //     //if you get here you have connected to the WiFi    
    //     Serial.println("connected...yeey :)");
    // }
 
}
 
void loop() {
    // put your main code here, to run repeatedly:   
}