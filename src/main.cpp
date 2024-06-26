#include <Arduino.h>
#include <LittleFS.h>
#include <DNSServer.h>
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
#define MAX_PIN_VALUE_SAVED 12

// mosquitto_pub -d -q 1 -h mqtt.thingsboard.cloud -p 1883 -t v1/devices/me/telemetry -u "v6IWWoRbN1t6sIGmYZOt" -m "{temperature:25}"
// #define fwVersion 2.1
#define fwVersion 1.0

const String apiFolder = "/storage/api/";
const String pinPath = "/storage/pin.json";
const String configPath = "/storage/config.json";
const String wifiPath = "/storage/wifi.json";
const String logPath = "/storage/log.txt";
const IPAddress apIP(192, 168, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const uint8_t pin[] = {2, 13, 14, 15, 19, 21, 25, 32, 33};
const uint8_t pinType[] = {0, 0, 0, 0, 0, 0, 0, 1, 1}; // 0 for digital, 1 for analog
const char *onenetServer = "47.91.203.178";            // Onenet IP address
const int onenetPort = 6002;                           // Onenet port number                               //variable of second data
const char *thingsboardServer = "thingsboard.cloud";
const int thingsboardPort = 1883;
const char *thingsboardUsername = "v6IWWoRbN1t6sIGmYZOt";
const char *thingsboardPassword = "";
const char *otaTopic = "v1/devices/me/attributes";

boolean APmode = false;
String wifiSSID = "";
String wifiPassword = "";

String onenetProductID = "";
String onenetDeviceID = "";
String onenetPassword = "";
int onenetTransferInterval = 10000;
int maltPinReadingInterval = 5000;
int maltPinRefreshingInterval = 10000;

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
uint8_t pinIndex[PIN_NUMBER];
uint8_t pinData[PIN_NUMBER][MAX_PIN_VALUE_SAVED];

// Networking
DNSServer dnsServer;
AsyncWebServer server(80);
WiFiClient wifiClient;
WiFiClient wifiClient2;
PubSubClient onenetClient(wifiClient);
PubSubClient thingsboardClient(wifiClient2);

bool isFirmwareUpgradeTriggered = false;

// device -> 11 pin -> onenetMQTT
char msgJson[100];
unsigned short json_len = 0;
char msg_buf[200];

String body_buf;
unsigned short body_len = 0;

// Log message to local file
void Log(String content)
{
    File file = LittleFS.open(logPath, "a+");
    file.print(content);
    file.close();
    delay(100);

    Serial.print(content);
}
void Logln(String content)
{
    const String _content = content + "\n";
    Log(_content);
}

// Pin Control
void setupPin()
{
    for (int i = 0; i < PIN_NUMBER; i++)
    {
        if (pinEnable[i])
        {
            Log("Set Pin ");
            if (pinReadWrite[i] == 0)
            {
                Log("Read, ");
            }
            else
            {
                Log("Write, ");
            }
            Logln(String(pin[i]));

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
}
void sendPinValue2Onenet()
{
    if (onenetClient.connected())
    {
        uint8_t pinCount = 0;
        String onenet_msg = "{";
        for (int i = 0; i < sizeof(pin); i++)
        {
            if (pinEnable[i] && pinReadWrite[i] == 0)
            {
                if (pinCount > 0)
                {
                    onenet_msg += ",";
                }
                uint8_t value;
                if (pinType[i] == 1)
                {
                    value = analogRead(pin[i]);
                }
                else
                {
                    value = digitalRead(pin[i]);
                }
                onenet_msg += "\"" + pinTopic[i] + "\":";
                onenet_msg += value;
                if (pinEventTriggerEnable[i])
                {
                    switch (pinEventTriggerType[i])
                    {
                    case 0:
                        if (value > pinEventTriggerValue[i])
                        {
                            onenet_msg += ",\"" + pinTopic[i] + " EventTrigger\": 1";
                        }
                        break;
                    case 1:
                        if (value == pinEventTriggerValue[i])
                        {
                            onenet_msg += ",\"" + pinTopic[i] + " EventTrigger\": 1";
                        }
                        break;
                    case 2:
                        if (value < pinEventTriggerValue[i])
                        {
                            onenet_msg += ",\"" + pinTopic[i] + " EventTrigger\": 1";
                        }
                        break;
                    }
                }
                pinCount += 1;
            }
        }
        onenet_msg += "}";
        const char *msgJson = onenet_msg.c_str();
        json_len = strlen(msgJson);                                           // length of msgJson
        msg_buf[0] = char(0x03);                                              // buffer of the data sent first part is 3
        msg_buf[1] = char(json_len >> 8);                                     // Second part is the 8 MSB of the data
        msg_buf[2] = char(json_len & 0xff);                                   // Third part is 8 LSB of the data
        memcpy(msg_buf + 3, msgJson, strlen(msgJson));                        // Forth part is the data in msgJson
        msg_buf[3 + strlen(msgJson)] = 0;                                     // Add a 0 at last
        onenetClient.publish("$dp", (uint8_t *)msg_buf, 3 + strlen(msgJson)); // Send data
        // Log("public message client:");
        // Log(msgJson);
    }
}
void controlPin(uint8_t pinControl, uint8_t pinValue)
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
void updatePinValue()
{
    for (uint8_t i = 0; i < PIN_NUMBER; i++)
    {
        if (pinEnable[i] && pinReadWrite[i] == 0)
        {
            uint8_t value = -1;
            if (pinType[i] == 0)
            {
                value = digitalRead(pin[i]);
            }
            else
            {
                value = analogRead(pin[i]);
            }
            pinData[i][pinIndex[i]] = value;
            pinIndex[i] = (pinIndex[i] + 1) % MAX_PIN_VALUE_SAVED;
        }
    }
}
String pinValue2str()
{
    String msgContent = "{";
    for (uint8_t i = 0; i < sizeof(pin); i++)
    {

        if (msgContent.length() > 1)
        {
            msgContent += ",";
        }
        msgContent += "\"" + String(pin[i]) + "\":[";
        if (pinEnable[i] && pinReadWrite[i] == 0)
        {

            String row = "";
            for (uint8_t valueCount = 0; valueCount < MAX_PIN_VALUE_SAVED; valueCount++)
            {
                uint8_t valueIndex = (valueCount + pinIndex[i]) % MAX_PIN_VALUE_SAVED;
                row += String(pinData[i][valueIndex]);
                if (valueCount != MAX_PIN_VALUE_SAVED - 1)
                {
                    row += ",";
                }
            }
            msgContent += row;
        }
        msgContent += "]";
    }
    msgContent += "}";
    return msgContent;
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
    String msgContent = "{\"onenetProductID\":\"";
    msgContent += onenetProductID;

    msgContent += "\", \"onenetDeviceID\":\"";
    msgContent += onenetDeviceID;

    msgContent += "\", \"onenetPassword\":\"";
    msgContent += onenetPassword;

    msgContent += "\", \"onenetTransferInterval\":\"";
    msgContent += String(onenetTransferInterval);

    msgContent += "\", \"maltPinReadingInterval\":\"";
    msgContent += String(maltPinReadingInterval);

    msgContent += "\", \"maltPinRefreshingInterval\":\"";
    msgContent += String(maltPinRefreshingInterval);

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
        Logln("Create Pin File");
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
            Log("Parsing failed: ");
            Logln(error.c_str());
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
        Logln("Create Config File");
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
            Log("Parsing failed: ");
            Logln(error.c_str());
            return "Error on Parsing";
        }
        onenetProductID = doc["onenetProductID"].as<String>();
        onenetDeviceID = doc["onenetDeviceID"].as<String>();
        onenetPassword = doc["onenetPassword"].as<String>();
        onenetTransferInterval = doc["onenetTransferInterval"].as<int>();
        maltPinReadingInterval = doc["maltPinReadingInterval"].as<int>();
        maltPinRefreshingInterval = doc["maltPinRefreshingInterval"].as<int>();
    }
    return configContent;
}
String readWiFi()
{
    String wifiContent;
    if (!LittleFS.exists(wifiPath))
    {
        // create file if not exist
        Logln("Create WiFi File");
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
            Log("Parsing failed: ");
            Logln(error.c_str());
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
    Logln("Start FirmwareUpdate");
    Logln(fwURL);

    WiFiClient client;
    // client.setInsecure();
    t_httpUpdate_return ret = httpUpdate.update(client, fwURL);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Log("HTTP_UPDATE_FAILD Error");
        Log(String(httpUpdate.getLastError()));
        Logln(httpUpdate.getLastErrorString().c_str());
        // Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Logln("HTTP_UPDATE_NO_UPDATES");
        break;
    case HTTP_UPDATE_OK:
        Logln("HTTP_UPDATE_OK");
        break;
    }
}

void handleSerialCommand(String cmd)
{
    if (cmd.c_str() == "restart")
    {
        ESP.restart();
        return;
    }

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
    controlPin(pinControl, pinValue);
}
void onenetCallback(char *topic, byte *payload, unsigned int length)
{
    Logln("-----------------------");
    Log("Onenet Message arrived in topic: ");
    Logln(topic);
    Logln("Message: ");
    String buf = "";
    for (int i = 0; i < length; i++)
    {
        buf += (char)payload[i];
    }
    Logln(buf);
    Logln("-----------------------");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        Log("Parsing failed: ");
        Logln(error.c_str());
        return;
    }
    JsonArray array = doc.as<JsonArray>();
    for (int i = 0; i < array.size(); i++)
    {
        JsonObject obj = array[i];
        const uint8_t pinControl = obj["pin"].as<uint8_t>();
        const uint8_t pinValue = obj["value"].as<uint8_t>();
        controlPin(pinControl, pinValue);
    }
}
void thingsboardCallback(char *topic, byte *payload, unsigned int length)
{
    if (strcmp(topic, otaTopic) == 0)
    {
        Logln("-----------------------");
        Log("Thingsboard Topic: ");
        Logln(topic);
        Logln("Message: ");
        String buf = "";
        for (int i = 0; i < length; i++)
        {
            buf += (char)payload[i];
        }
        Logln(buf);
        Logln("-----------------------");
        if (!isFirmwareUpgradeTriggered)
        {
            isFirmwareUpgradeTriggered = true;
            Logln("Firmware upgrade triggered!");
            firmwareUpdate();
        }
    }
}
void handleNotFound(AsyncWebServerRequest *request)
{
    Logln("NOT FOUND");
    request->send(404, "text/plain", "Not found");
}
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    Log("Import ... ");
    if (!index)
    {
        Log("Trigger Index");
        request->_tempFile = LittleFS.open("/storage/" + filename, "w");
    }
    if (len)
    {
        Log("Trigger len");
        request->_tempFile.write(data, len);
    }

    if (final)
    {
        Log("Trigger Final");
        request->_tempFile.close();

        String filePath = "/storage/";
        filePath.concat(filename);
        Log(filePath);

        String fileContent = readJson(filePath.c_str());
        Log(fileContent);

        StaticJsonDocument<4096> doc;
        DeserializationError error = deserializeJson(doc, fileContent);
        if (error)
        {
            Log("Parsing failed: ");
            Log(error.c_str());
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
                Logln("Connecting to WiFi...");
            }
            else
            {
                Logln("Connected to WiFi");
                Log("IP Address: ");
                Logln(WiFi.SSID());
                return true;
            }
        }
        Logln("Failed to connect WiFi");
    }
    return false;
}
boolean onenetConnect()
{
    Logln("Onenet Connecting ...");
    if (onenetProductID.length() <= 0 || onenetDeviceID.length() <= 0 || onenetPassword.length() <= 0)
    {
        Logln("Missing Onenet Parameter.");
        return false;
    }

    onenetClient.setServer(onenetServer, onenetPort);
    int retry = 0;
    while (!onenetClient.connected())
    {
        if (onenetClient.connect(onenetDeviceID.c_str(), onenetProductID.c_str(), onenetPassword.c_str()))
        {
            Logln("Onenet Connected");
            onenetClient.setCallback(onenetCallback);
            return true;
        }
        else
        {
            Log("Retry [");
            Log(String(retry));
            Log("] Failed, rc=");
            Log(String(onenetClient.state()));
            Logln(", Retrying in 3 seconds");
            retry += 1;
            if (retry >= 5)
            {
                Logln("Attempted Connection to Onenet for 5 times. Retry later.");
                break;
            }
            delay(3000);
        }
    }
    return false;
}
boolean thingsboardConnect()
{
    Logln("Thingsboard Connecting ...");
    thingsboardClient.setServer(thingsboardServer, thingsboardPort);
    int retry = 0;
    while (!thingsboardClient.connected())
    {
        if (thingsboardClient.connect("ESP32Client", thingsboardUsername, thingsboardPassword))
        {
            Logln("Thingsboard Connected");
            thingsboardClient.subscribe(otaTopic);
            thingsboardClient.setCallback(thingsboardCallback);
            return true;
        }
        else
        {
            Log("Retry [");
            Log(String(retry));
            Log("] Failed, rc=");
            Log(String(thingsboardClient.state()));
            Logln(", Retrying in 3 seconds");
            retry += 1;
            if (retry >= 5)
            {
                Logln("Attempted Connection to Thingsboard for 5 times.");
                break;
            }
            delay(3000);
        }
    }
    return false;
}

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

    Log("Reading File ... ");
    Logln(filePath);

    String ret;

    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Logln("Failed to open file");
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
              AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
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

                    Log("Save API ... ");
                    Logln(jsonPath);

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
            Log("Save Pin ... ");
            Logln(pinPath);
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
        onenetProductID = getQueryParam(request, "onenetProductID");
        onenetDeviceID = getQueryParam(request, "onenetDeviceID");
        onenetPassword = getQueryParam(request, "onenetPassword");
        onenetTransferInterval = getQueryParam(request, "onenetTransferInterval").toInt();
        maltPinReadingInterval = getQueryParam(request, "maltPinReadingInterval").toInt();
        maltPinRefreshingInterval = getQueryParam(request, "maltPinRefreshingInterval").toInt();

        const String configContent = config2str();
        Log("Save Config ... ");
        Logln(configPath);
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
        Log("Save Wifi ... ");
        Logln(wifiPath);
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
                Logln("Export ... ");
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

    server.on("/pin/value", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", pinValue2str()); });

    // server.on("/tailwindcss_3.3.3.js", HTTP_GET, [](AsyncWebServerRequest *request)
    //           {
    //             String js = readFileContent("/tailwindcss_3.3.3.js");
    //     request->send(200, "text/javascript", js); });

    server.on("/chart.umd.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                String js = readFileContent("/chart.umd.min.js");
        request->send(200, "text/javascript", js); });

    server.onNotFound(handleNotFound);
}

void setup()
{
    Serial.begin(9600);
    // Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    if (!LittleFS.begin())
    {
        Logln("An Error has occurred while mounting LittleFS");
        return;
    }

    WiFi.disconnect();
    pinMode(CONFIG_TRIGGER_PIN, INPUT);
    if (digitalRead(CONFIG_TRIGGER_PIN) == LOW)
    {
        Logln("AP Mode");
        WiFi.mode(WIFI_AP);
        APmode = true;
    }
    else
    {
        Logln("STA Mode");
        WiFi.mode(WIFI_STA);
    }

    readPin();
    readConfig();
    readWiFi();

    setupPin();

    if (APmode)
    {
        Logln("Start Config Portal");

        const String apName = "ESP32-" + WiFi.macAddress();
        WiFi.softAP(apName);
        WiFi.softAPConfig(apIP, apIP, subnetMask);

        configureWebServer();

        dnsServer.start(53, "*", WiFi.softAPIP());
        // server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

        Log("AP IP address: ");
        Logln(WiFi.softAPSSID());

        server.on("*", HTTP_ANY, [](AsyncWebServerRequest *request)
                  { request->redirect("http://" + apIP.toString()); });
        server.begin();
    }
    else
    {

        wifiConnect();
        if (WiFi.status() == WL_CONNECTED)
        {
            // has wifi, connect to onenet onenetMQTT
            // onenetConnect();
            thingsboardConnect();
        }

        Log("Active firmware version:");
        Logln(String(fwVersion));
    }
    Logln("=================================");
}

unsigned long previousOnenetTransferMillis = 0;
unsigned long previousPinReadMillis = 0;
String serialBuffer;
void loop()
{
    // while (Serial2.available())
    // {
    //     const int d = Serial2.read();
    //     Log((char)d);
    //     Serial2.write(d);
    // }
    unsigned long currentMillis = millis();

    while (Serial.available())
    {
        serialBuffer += Serial.readString();

        if (serialBuffer.indexOf('\n') != -1)
        {
            Log("ECHO: ");
            Logln(serialBuffer);
            handleSerialCommand(serialBuffer);
            serialBuffer = "";
        }
    }

    if ((currentMillis - previousPinReadMillis) >= maltPinReadingInterval)
    {
        updatePinValue();
        previousPinReadMillis = currentMillis;
    }

    if (APmode)
    {
        dnsServer.processNextRequest();
        return;
    }

    if ((currentMillis - previousOnenetTransferMillis) >= onenetTransferInterval)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Logln("Wifi DC ... reconnecting");
            wifiConnect();
        }
        else
        {
            Serial.print(".");

            // if (!onenetClient.connected())
            // {
            //     onenetConnect();
            // }
            // else
            // {
            //     sendPinValue2Onenet();
            // }

            if (!thingsboardClient.connected())
            {
                thingsboardConnect();
            }
        }
        previousOnenetTransferMillis = currentMillis;
    }
    onenetClient.loop();
    thingsboardClient.loop();
    delay(100);
}
