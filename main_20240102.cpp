#include <Arduino.h>
#include <LittleFS.h>
// #include <DNSServer.h>
#include <fileHelper.h>

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <secrets.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define PIN_NUMBER 9
#define CONFIG_TRIGGER_PIN 27
#define WIFICONNECT_RETRY_TIMES 5
#define RXD2 9
#define TXD2 10

#define fwVersion 1.0

const String apiFolder = "/storage/api/";
const String pinPath = "/storage/pin.json";
const String configPath = "/storage/config.json";
const String wifiPath = "/storage/wifi.json";
const IPAddress apIP(192, 168, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const uint8_t pin[] = {2, 13, 14, 15, 19, 21, 25, 32, 33};
const uint8_t pinType[] = {0, 0, 0, 0, 0, 0, 0, 1, 1}; // 0 for digital, 1 for analog
const char *onenetServer = "47.91.203.178";            // Onenet IP address
const int onenetPort = 6002;                           // Onenet port number                               //variable of second data
const char *thingsboardServer = "thingsboard.cloud";
const int thingsboardPort = 1883;
const char *thingsboardUsername = "vNqysU5VuhevcJy1wBLv";
const char *thingsboardPassword = "";
const char *otaTopic = "v1/devices/me/attributes";

boolean APmode = false;
String wifiSSID = "";
String wifiPassword = "";

String onenetProductID = "";
String onenetDeviceID = "";
String onenetPassword = "";
String onenetTransferInterval = "10000";

// pin-related => re-write into a class
uint8_t pinReadWrite[] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // 0 for read, 1 for write
boolean pinEnable[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
String pinTopic[] = {
    "Pin 2",
    "Pin 13",
    "Pin 14",
    "Pin 15",
    "Pin 19",
    "Pin 21",
    "Pin 25",
    "Pin 32",
    "Pin 33",
};
boolean pinEventTriggerEnable[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t pinEventTriggerType[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t pinEventTriggerValue[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// DNSServer dnsServer;
AsyncWebServer server(80);
WiFiClient wifiClient;
PubSubClient thingsboardClient(wifiClient);
// PubSubClient onenetClient(wifiClient);
// WiFiClient wifiClient2;

bool isFirmwareUpgradeTriggered = false;

// device -> 11 pin -> onenetMQTT
char msgJson[100];
unsigned short json_len = 0;
char msg_buf[200];

String body_buf;
unsigned short body_len = 0;

void pinTrigger(uint8_t pinControl, uint8_t pinValue)
{
    for (int i = 0; i < PIN_NUMBER; i++)
    {
        if (pin[i] == pinControl && pinEnable[i])
        {
            if (pinType[i] == 1)
            {
                analogWrite(pinControl, pinValue);
            }
            else
            {
                digitalWrite(pinControl, pinValue);
            }
        }
    }
}

String wifi2str()
{
    String msgContent = "{\"ssid\":\"";
    msgContent += wifiSSID;
    msgContent += "\", \"password\":\"";
    msgContent += wifiPassword;
    msgContent += "\"}";
    return msgContent;
}
String config2str()
{
    String msgContent = "{\"productID\":\"";
    msgContent += onenetProductID;

    msgContent += "\", \"deviceID\":\"";
    msgContent += onenetDeviceID;

    msgContent += "\", \"password\":\"";
    msgContent += onenetPassword;

    msgContent += "\", \"interval\":\"";
    msgContent += onenetTransferInterval;

    msgContent += "\"}";

    return msgContent;
}
String configFilestr(File file)
{
    String ret;
    ret += "\"";
    ret += file.name();
    ret += "-";
    ret += file.size();
    ret += "\"";

    return ret;
}
String readPin()
{
    String pinContent;
    if (!LittleFS.exists(pinPath))
    {
        Serial.println("Create Pin File");
        pinContent = "[";

        for (int i = 0; i < PIN_NUMBER; i++)
        {
            pinContent += "{";

            pinContent += "\"pin\": ";
            pinContent += pin[i];
            pinContent += ",";

            pinContent += "\"pinTopic\": \"";
            pinContent += pinTopic[i];
            pinContent += "\",";

            pinContent += "\"pinEnable\": ";
            pinContent += pinEnable[i];
            pinContent += ",";

            pinContent += "\"pinReadWrite\": ";
            pinContent += pinReadWrite[i];
            pinContent += ",";

            pinContent += "\"pinEventTriggerEnable\": ";
            pinContent += pinEventTriggerEnable[i];
            pinContent += ",";

            pinContent += "\"pinEventTriggerType\": ";
            pinContent += pinEventTriggerType[i];
            pinContent += ",";

            pinContent += "\"pinEventTriggerValue\": ";
            pinContent += pinEventTriggerValue[i];

            pinContent += "}";

            if (i < sizeof(pin) - 1)
            {
                pinContent += ",";
            }
        }
        pinContent += "]";
        saveJson(pinContent.c_str(), pinPath.c_str());
    }
    else
    {
        pinContent = readJson(pinPath.c_str());

        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, pinContent);
        if (error)
        {
            Serial.print("Parsing failed: ");
            Serial.println(error.c_str());
            return "Error on Parsing";
        }

        JsonArray array = doc.as<JsonArray>();
        for (int i = 0; i < array.size(); i++)
        {
            JsonObject obj = array[i];

            pinEnable[i] = obj["pinEnable"].as<boolean>();
            pinReadWrite[i] = obj["pinReadWrite"].as<uint8_t>();
            pinTopic[i] = obj["pinTopic"].as<String>();
            pinEventTriggerEnable[i] = obj["pinEventTriggerEnable"].as<boolean>();
            pinEventTriggerType[i] = obj["pinEventTriggerType"].as<uint8_t>();
            pinEventTriggerValue[i] = obj["pinEventTriggerValue"].as<uint8_t>();
        }
    }
    return pinContent;
}
String readConfig()
{
    String configContent;
    if (!LittleFS.exists(configPath))
    {
        // create file if not exist
        Serial.println("Create Config File");
        configContent = config2str();
        saveJson(configContent.c_str(), configPath.c_str());
    }
    else
    {
        // read file to config
        configContent = readJson(configPath.c_str());

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, configContent);
        if (error)
        {
            Serial.print("Parsing failed: ");
            Serial.println(error.c_str());
            return "Error on Parsing";
        }
        onenetProductID = doc["productID"].as<String>();
        onenetDeviceID = doc["deviceID"].as<String>();
        onenetPassword = doc["password"].as<String>();
        onenetTransferInterval = doc["interval"].as<String>();
    }
    return configContent;
}
String readWiFi()
{
    String wifiContent;
    if (!LittleFS.exists(wifiPath))
    {
        // create file if not exist
        Serial.println("Create WiFi File");
        wifiContent = wifi2str();
        saveJson(wifiContent.c_str(), wifiPath.c_str());
    }
    else
    {
        // read file to config
        wifiContent = readJson(wifiPath.c_str());

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, wifiContent);
        if (error)
        {
            Serial.print("Parsing failed: ");
            Serial.println(error.c_str());
            return "Error on Parsing";
        }
        wifiSSID = doc["ssid"].as<String>();
        wifiPassword = doc["password"].as<String>();
    }
    return wifiContent;
}

bool FirmwareVersionCheck()
{
    // check version here
    return 1;
}
void firmwareUpdate()
{
    Serial.println("Start FirmwareUpdate");
    Serial.println(fwURL);
    WiFiClient client;
    t_httpUpdate_return ret = httpUpdate.update(client, fwURL);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
}

void handleSerialCommand(String cmd)
{
    const String delimiter = " ";
    const uint8_t index = cmd.indexOf(delimiter);
    if (index == -1)
    {
        return;
    }
    String word = cmd.substring(0, index);
    String other = cmd.substring(index + 1);
    uint8_t pinControl = word.toInt();
    uint8_t pinValue = other.toInt();
    if (pinControl == 0)
    {
        return;
    }
    pinTrigger(pinControl, pinValue);
}
// void onenetCallback(char *topic, byte *payload, unsigned int length)
// {
//     Serial.println("-----------------------");
//     Serial.print("Onenet Message arrived in topic: ");
//     Serial.println(topic);
//     Serial.print("Message: ");
//     String buf = "";
//     for (int i = 0; i < length; i++)
//     {
//         buf += (char)payload[i];
//     }
//     Serial.println(buf);
//     Serial.println("-----------------------");
//     StaticJsonDocument<256> doc;
//     DeserializationError error = deserializeJson(doc, buf);
//     if (error)
//     {
//         Serial.print("Parsing failed: ");
//         Serial.println(error.c_str());
//         return;
//     }
//     JsonArray array = doc.as<JsonArray>();
//     for (int i = 0; i < array.size(); i++)
//     {
//         JsonObject obj = array[i];
//         const uint8_t pinControl = obj["pin"].as<uint8_t>();
//         const uint8_t pinValue = obj["value"].as<uint8_t>();
//         pinTrigger(pinControl, pinValue);
//     }
// }

void thingsboardCallback(char *topic, byte *payload, unsigned int length)
{
    if (strcmp(topic, otaTopic) == 0)
    {
        Serial.println("-----------------------");
        Serial.print("Thingsboard Topic: ");
        Serial.println(topic);
        Serial.print("Message: ");
        String buf = "";
        for (int i = 0; i < length; i++)
        {
            buf += (char)payload[i];
        }
        Serial.println(buf);

        if (!isFirmwareUpgradeTriggered)
        {
            isFirmwareUpgradeTriggered = true;
            Serial.println("Firmware upgrade triggered!");
            firmwareUpdate();
        }
    }
}
void handleNotFound(AsyncWebServerRequest *request)
{
    Serial.println("NOT FOUND");
    request->send(404, "text/plain", "Not found");
}
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    Serial.println(filename);
    Serial.println(index);
    Serial.println(len);
    Serial.println(final);

    Serial.println("Import ... ");
    if (!index)
    {
        Serial.println("Trigger Index");
        request->_tempFile = LittleFS.open("/storage/" + filename, "w");
    }
    if (len)
    {
        Serial.println("Trigger len");
        request->_tempFile.write(data, len);
    }

    if (final)
    {
        Serial.println("Trigger Final");
        request->_tempFile.close();

        String filePath = "/storage/";
        filePath.concat(filename);
        Serial.println(filePath);

        String fileContent = readJson(filePath.c_str());
        Serial.println(fileContent);

        StaticJsonDocument<4096> doc;
        DeserializationError error = deserializeJson(doc, fileContent);
        if (error)
        {
            Serial.print("Parsing failed: ");
            Serial.println(error.c_str());
            return;
        }
        const String pinContent = doc["pin"].as<String>();
        saveJson(pinContent.c_str(), pinPath.c_str());

        const String configContent = doc["config"].as<String>();
        saveJson(configContent.c_str(), configPath.c_str());

        const String wifiContent = doc["wifi"].as<String>();
        saveJson(wifiContent.c_str(), wifiPath.c_str());

        delay(100);

        readPin();
        readConfig();
        readWiFi();

        request->send(200);
    }
}

boolean wifiConnect()
{
    if (wifiSSID.length() > 0 && wifiPassword.length() > 0)
    {
        delay(100);

        WiFi.begin(wifiSSID, wifiPassword);

        for (int i = 0; i < WIFICONNECT_RETRY_TIMES; i++)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                delay(1000);
                Serial.println("Connecting to WiFi...");
            }
            else
            {
                Serial.println("Connected to WiFi");
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());
                return true;
            }
        }
        Serial.println("Failed to connect WiFi");
    }
    return false;
}
// boolean onenetConnect()
// {
//     Serial.println("Onenet MQTT Connecting ...");
//     onenetClient.setServer(onenetServer, onenetPort);
//     if (onenetProductID.length() <= 0 || onenetDeviceID.length() <= 0 || onenetPassword.length() <= 0)
//     {
//         Serial.println("Missing Onenet Parameter.");
//         return false;
//     }
//     onenetClient.connect(onenetDeviceID.c_str(), onenetProductID.c_str(), onenetPassword.c_str());
//     onenetClient.setCallback(onenetCallback);
//     if (onenetClient.connected())
//     {
//         Serial.printf("OneNet connected to %s\n", onenetDeviceID);
//         return true;
//     }
//     return false;
// }

boolean thingsboardConnect()
{
    Serial.println("Thingsboard MQTT Connecting ...");
    thingsboardClient.setServer(thingsboardServer, thingsboardPort);
    while (!thingsboardClient.connected())
    {
        Serial.println("Connecting to MQTT ...");
        if (thingsboardClient.connect("ESP32Client", thingsboardUsername, thingsboardPassword))
        {
            Serial.println("Connected to MQTT");
            thingsboardClient.subscribe(otaTopic);
            thingsboardClient.setCallback(thingsboardCallback);
            return true;
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(thingsboardClient.state());
            Serial.println(", Retrying in 5 seconds");
            delay(5000);
        }
        delay(1000);
    }
    return false;
}

// void sendPinValue()
// {
//     if (onenetClient.connected())
//     {
//         uint8_t pinCount = 0;
//         String onenet_msg = "{";
//         for (int i = 0; i < sizeof(pin); i++)
//         {
//             if (pinEnable[i] && pinReadWrite[i] == 0)
//             {
//                 if (pinCount > 0)
//                 {
//                     onenet_msg += ",";
//                 }
//                 uint8_t value;
//                 if (pinType[i] == 1)
//                 {
//                     value = analogRead(pin[i]);
//                 }
//                 else
//                 {
//                     value = digitalRead(pin[i]);
//                 }
//                 onenet_msg += "\"" + pinTopic[i] + "\":";
//                 onenet_msg += value;
//                 if (pinEventTriggerEnable[i])
//                 {
//                     switch (pinEventTriggerType[i])
//                     {
//                     case 0:
//                         if (value > pinEventTriggerValue[i])
//                         {
//                             onenet_msg += ",\"" + pinTopic[i] + " EventTrigger\": 1";
//                         }
//                         break;
//                     case 1:
//                         if (value == pinEventTriggerValue[i])
//                         {
//                             onenet_msg += ",\"" + pinTopic[i] + " EventTrigger\": 1";
//                         }
//                         break;
//                     case 2:
//                         if (value < pinEventTriggerValue[i])
//                         {
//                             onenet_msg += ",\"" + pinTopic[i] + " EventTrigger\": 1";
//                         }
//                         break;
//                     }
//                 }
//                 pinCount += 1;
//             }
//         }
//         onenet_msg += "}";
//         const char *msgJson = onenet_msg.c_str();
//         json_len = strlen(msgJson);                    // length of msgJson
//         msg_buf[0] = char(0x03);                       // buffer of the data sent first part is 3
//         msg_buf[1] = char(json_len >> 8);              // Second part is the 8 MSB of the data
//         msg_buf[2] = char(json_len & 0xff);            // Third part is 8 LSB of the data
//         memcpy(msg_buf + 3, msgJson, strlen(msgJson)); // Forth part is the data in msgJson
//         msg_buf[3 + strlen(msgJson)] = 0;              // Add a 0 at last
//         onenetClient.publish("$dp", (uint8_t *)msg_buf, 3 + strlen(msgJson)); // Send data
//         // Serial.print("public message client:");
//         // Serial.println(msgJson);
//     }
// }

int listDirectoryCount(String path)
{
    int fileCount = 0;
    File root = LittleFS.open(path);
    File file = root.openNextFile();
    while (file)
    {
        fileCount++;
        file = root.openNextFile();
    }
    return fileCount;
}
String listDirectory(String path)
{
    File root = LittleFS.open(path);
    File file = root.openNextFile();
    String ret = "{";
    while (file)
    {
        ret += "\"";
        ret += file.name();
        ret += "\":";

        if (file.isDirectory())
        {
            String nextPath = path + "/" + file.name();
            ret += listDirectory(nextPath);
        }
        else
        {
            ret += configFilestr(file);
        }

        file = root.openNextFile();
        if (file)
        {
            ret += ",";
        }
    }

    ret += "}";
    return ret;
}
String readFileContent(String filePath)
{

    Serial.print("Reading File ... ");
    Serial.println(filePath);

    String ret;

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("Failed to open file");
        return ret;
    }

    while (file.available())
    {
        ret += (char)file.read();
    }

    file.close();
    return ret;
}

String getQueryParam(AsyncWebServerRequest *request, String targetQueryKey)
{
    String ret;
    int paramsNr = request->params();
    for (int i = 0; i < paramsNr; i++)
    {
        AsyncWebParameter *p = request->getParam(i);
        String queryKey = p->name();

        if (strcmp(queryKey.c_str(), targetQueryKey.c_str()) == 0)
        {
            return p->value();
        }
    }
    return ret;
}
void configureWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html", "text/html");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response); });
    server.on("/storage", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                int paramsNr = request->params();
                if (paramsNr > 0){
                    String filePath = "/storage/"+getQueryParam(request, "filepath");
                    String fileContent = readFileContent(filePath);
                    request->send(200, "text/plain", fileContent);
                }else
                {
                    String ret = listDirectory("/storage");
                    request->send(200, "text/plain", ret);
        } });
    server.on(
        "/api/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
                    String apiName = getQueryParam(request, "apiName");

                    String apiJson;
                    for (size_t i = 0; i < len; i++) {
                        apiJson += char(data[i]);
                    }

                    int fileIndex = listDirectoryCount(apiFolder) + 1;
                    String jsonPath = apiFolder + "api_"+fileIndex+"_"+apiName+".json"; //"_"+datetime+

                    Serial.print("Save API ... ");
                    Serial.println(jsonPath);

                    saveJson(apiJson.c_str(), jsonPath.c_str());
                    request->send(200, "text/plain", jsonPath); });
    server.on("/api/load", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String ret = listDirectory("/storage/api/");
        request->send(200, "text/plain", ret); });

    server.on(
        "/pin/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
        if (index == 0)
        {
            body_len = 0;
            body_buf = "";
        }

        for (size_t i = 0; i < len; i++)
        {
            body_buf += char(data[i]);
        }

        body_len += len;

        if (body_len == total)
        {
            Serial.println("Save PinConfig ... ");
            saveJson(body_buf.c_str(), pinPath.c_str());
            request->send(200, "text/plain", pinPath);

            // ESP.restart();
        } });
    // server.on("/pin", HTTP_GET)
    server.on(
        "/config", HTTP_GET, [](AsyncWebServerRequest *request)
        {
    int paramsNr = request->params();
    if (paramsNr > 0)
    {
        // save config
        onenetProductID = getQueryParam(request, "productID");
        onenetDeviceID = getQueryParam(request, "deviceID");
        onenetPassword = getQueryParam(request, "password");
        onenetTransferInterval = getQueryParam(request, "interval");

        const String configContent = config2str();
        saveJson(configContent.c_str(), configPath.c_str());

        request->send(200, "text/plain", "OK");
    } else {
        // load config
        const String msg = config2str();
        request->send(200, "text/plain", msg);
    } });
    server.on(
        "/wifi", HTTP_GET, [](AsyncWebServerRequest *request)
        {
    int paramsNr = request->params();
    if (paramsNr > 0)
    {
        // save wifi
        wifiSSID = getQueryParam(request, "ssid");
        wifiPassword = getQueryParam(request, "password");
        const String wifiContent = wifi2str();
        saveJson(wifiContent.c_str(), wifiPath.c_str());

        request->send(200, "text/plain", "OK");
    } else {
        // load wifi
        const String msg = wifi2str();
        request->send(200, "text/plain", msg);
    } });
    server.on("/export", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                Serial.println("Export ... ");
                // Open the files
                File pinFile = LittleFS.open(pinPath, "r");
                File configFile = LittleFS.open(configPath, "r");
                File wifiFile = LittleFS.open(wifiPath, "r");
                const char* exportPath = "/export.json";

                if (!pinFile || !configFile || !wifiFile)
                {
                    request->send(404, "text/plain", "File Not Found");
                    return;
                }

                File exportFile = LittleFS.open(exportPath, "w");
                if (!exportFile)
                {
                    request->send(500, "text/plain", "Failed to create combined file");
                    return;
                }

                exportFile.print("{\"pin\":");
                while (pinFile.available())
                {
                    exportFile.write(pinFile.read());
                }
                exportFile.print(",");

                exportFile.print("\"config\":");
                while (configFile.available())
                {
                    exportFile.write(configFile.read());
                }
                exportFile.print(",");

                exportFile.print("\"wifi\":");
                while (wifiFile.available())
                {
                    exportFile.write(wifiFile.read());
                }
                exportFile.print("}");

                exportFile.close();
                pinFile.close();
                configFile.close();
                wifiFile.close();

                request->send(LittleFS, exportPath, "text/plain"); });
    server.on(
        "/import", HTTP_POST, [](AsyncWebServerRequest *request) {},
        handleUpload);

    // server.on("/tailwindcss_3.3.3.js", HTTP_GET, [](AsyncWebServerRequest *request)
    //           {
    //             String js = readFileContent("/tailwindcss_3.3.3.js");
    //     request->send(200, "text/javascript", js); });

    server.onNotFound(handleNotFound);
}

void setup()
{
    Serial.begin(115200);
    // Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        // return;
    }

    WiFi.disconnect();
    pinMode(CONFIG_TRIGGER_PIN, INPUT);
    if (digitalRead(CONFIG_TRIGGER_PIN) == LOW)
    {
        Serial.println("AP Mode");
        WiFi.mode(WIFI_AP);
        APmode = true;
    }
    else
    {
        Serial.println("STA Mode");
        WiFi.mode(WIFI_STA);
    }

    readPin();
    readConfig();
    readWiFi();

    if (APmode)
    {
        Serial.println("Start Config Portal");

        const String apName = "ESP32-" + WiFi.macAddress();
        WiFi.softAP(apName);
        WiFi.softAPConfig(apIP, apIP, subnetMask);

        configureWebServer();

        // dnsServer.start(53, "*", WiFi.softAPIP());
        // server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());

        server.on("*", HTTP_ANY, [](AsyncWebServerRequest *request)
                  { request->redirect("http://" + apIP.toString()); });
        server.begin();
    }
    else
    {
        for (int i = 0; i < PIN_NUMBER; i++)
        {
            if (pinEnable[i])
            {
                Serial.print("Set Pin ");
                if (pinReadWrite[i] == 0)
                {
                    Serial.print("Read, ");
                }
                else
                {
                    Serial.print("Write, ");
                }
                Serial.println(pin[i]);
                if (pinReadWrite[i] == 0)
                {
                    pinMode(pin[i], INPUT);
                }
                else
                {
                    pinMode(pin[i], OUTPUT);
                    digitalWrite(pin[i], 0);
                }
            }
        }

        wifiConnect();
        if (WiFi.status() == WL_CONNECTED)
        {
            // has wifi, connect to onenet onenetMQTT
            // onenetConnect();
            thingsboardConnect();
        }

        Serial.print("Active firmware version:");
        Serial.println(fwVersion);
    }
    Serial.println("=================================");
}

unsigned long previousMillis = 0;
String serialBuffer;
void loop()
{
    // while (Serial2.available())
    // {
    //     const int d = Serial2.read();
    //     Serial.print((char)d);
    //     Serial2.write(d);
    // }

    while (Serial.available())
    {
        serialBuffer += Serial.readString();

        if (serialBuffer.indexOf('\n') != -1)
        {
            Serial.print("ECHO: ");
            Serial.print(serialBuffer);
            handleSerialCommand(serialBuffer);
            serialBuffer = "";
        }
    }

    if (APmode)
    {
        // dnsServer.processNextRequest();
        return;
    }

    unsigned long currentMillis = millis();
    if ((currentMillis - previousMillis) >= onenetTransferInterval.toInt())
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Wifi DC ... reconnecting");
            wifiConnect();
        }
        else
        {
            Serial.print(".");

            // if (!onenetClient.connected())
            // {
            //     onenetConnect();
            // }
            // sendPinValue();

        }
        previousMillis = currentMillis;
    }
    // onenetClient.loop();
    
    if (!thingsboardClient.connected())
    {
        thingsboardConnect();
    }
    thingsboardClient.loop();
    delay(100);
}
