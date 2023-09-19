#include <LittleFS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "fileHelper.h"
#include <ESPAsyncWebServer.h>
#include "PubSubClient.h"
// #include <ArduinoJson.h>

#define PIN_NUMBER 11
const String apiFolder = "/storage/api/";
const String configPath = "/storage/pinconfig.json";

// pin-related => re-write into a class
uint8_t pins[] = {2, 9, 10, 13, 14, 15, 19, 21, 25, 32, 33};
boolean pinsEnable[] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t pinsMode[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
String pinsName[] = {
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
    "Pin",
};
uint16_t pinsInterval[] = {10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000};
boolean pinsOnenetEnable[] = {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
String pinsOnenetProductID[] = {"16001514", "", "", "", "", "", "", "", "", "", ""};
String pinsOnenetDeviceID[] = {"161194727", "", "", "", "", "", "", "", "", "", ""};
String pinsOnenetPassword[] = {"dht11", "", "", "", "", "", "", "", "", "", ""};

// const size_t pinConfigCapacity = JSON_ARRAY_SIZE(PIN_NUMBER) + PIN_NUMBER * JSON_OBJECT_SIZE(9)+200;
// DynamicJsonDocument doc(pinConfigCapacity);

WiFiManager wm;
AsyncWebServer server(80);

const char *mqtt_server = "47.91.203.178"; // Onenet IP address
const int mqtt_port = 6002;                // port number                               //variable of second data

// WiFiClient espClient[PIN_NUMBER];
// PubSubClient client[PIN_NUMBER];

char dataTemplate[] = "{\"value\":%.2f}"; // Data template
char msgJson[100];
unsigned short json_len = 0;
char msg_buf[200];

String body_buf;
unsigned short body_len = 0;

String readPinConfig()
{
    String configContent;
    if (!LittleFS.exists(configPath))
    {
        Serial.println("Create Config File");
        configContent = "[";

        for (int i = 0; i < sizeof(pins); i++)
        {
            configContent += "{";

            configContent += "\"pin\": ";
            configContent += pins[i];
            configContent += ",";

            configContent += "\"name\": \"";
            configContent += pinsName[i];
            configContent += "\",";

            configContent += "\"pinEnable\": ";
            configContent += pinsEnable[i];
            configContent += ",";

            configContent += "\"pinMode\": ";
            configContent += pinsMode[i];
            configContent += ",";

            configContent += "\"interval\": ";
            configContent += pinsInterval[i];
            configContent += ",";

            configContent += "\"onenetEnable\": ";
            configContent += pinsOnenetEnable[i];
            configContent += ",";

            configContent += "\"productID\": \"";
            configContent += pinsOnenetProductID[i];
            configContent += "\",";

            configContent += "\"deviceID\": \"";
            configContent += pinsOnenetDeviceID[i];
            configContent += "\",";

            configContent += "\"password\": \"";
            configContent += pinsOnenetPassword[i];
            configContent += "\"";

            configContent += "}";

            if (i < sizeof(pins) - 1)
            {
                configContent += ",";
            }
        }
        configContent += "]";
        saveJson(configContent.c_str(), configPath.c_str());
    }
    else
    {
        configContent = readJson(configPath.c_str());
 
        // DeserializationError error = deserializeJson(doc, configContent);
        // if (error)
        // {
        //     Serial.print("Parsing failed: ");
        //     Serial.println(error.c_str());
        //     return "Error on Parsing";
        // }

        // JsonArray array = doc.as<JsonArray>();
        // for (int i = 0; i < array.size(); i++)
        // {
        //     JsonObject obj = array[i];

        //     const uint8_t pin = obj["pin"];
        //     const boolean pinEnable = obj["pinEnable"];
        //     const uint8_t pinMode = obj["pinMode"];
        //     const char *pinName = obj["pinName"];
        //     const uint16_t interval = obj["interval"];
        //     const boolean onenetEnable = obj["interval"];
        //     const String productID = obj["productID"];
        //     const String deviceID = obj["deviceID"];
        //     const String password = obj["password"];

        //     pins[i] = pin;
        //     pinsEnable[i] = pinEnable;
        //     pinsMode[i] = pinMode;
        //     pinsName[i] = pinName;
        //     pinsInterval[i] = interval;
        //     pinsOnenetEnable[i] = onenetEnable;
        //     pinsOnenetProductID[i] = productID;
        //     pinsOnenetDeviceID[i] = deviceID;
        //     pinsOnenetPassword[i] = password;
        // }
    }
    return configContent;
}
// void mqttConnect(uint8_t client_index)
// {
//     client[client_index].connect(pinsOnenetDeviceID[client_index].c_str(), pinsOnenetProductID[client_index].c_str(), pinsOnenetPassword[client_index].c_str());
//     if (client[client_index].connected())
//     {
//         Serial.printf("\n Pin %d OneNet is connected to %s !\n", pins[client_index], pinsOnenetDeviceID[client_index]);
//     }
//     // client.setCallback(callback);
// }
// void sendPinValue(uint8_t client_index)
// {
//     if (client[client_index].connected())
//     {
//         float v = analogRead(pins[client_index]);
//         snprintf(msgJson, 40, dataTemplate, v);        // Put the data in the template and pass to msgJson
//         json_len = strlen(msgJson);                    // length of msgJson
//         msg_buf[0] = char(0x03);                       // buffer of the data sent first part is 3
//         msg_buf[1] = char(json_len >> 8);              // Second part is the 8 MSB of the data
//         msg_buf[2] = char(json_len & 0xff);            // Third part is 8 LSB of the data
//         memcpy(msg_buf + 3, msgJson, strlen(msgJson)); // Forth part is the data in msgJson
//         msg_buf[3 + strlen(msgJson)] = 0;              // Add a 0 at last

//         client[client_index].publish("$dp", (uint8_t *)msg_buf, 3 + strlen(msgJson)); // Send data
//         Serial.print("public message client:");
//         Serial.println(msgJson);
//     }
// }

void notFound(AsyncWebServerRequest *request)
{
    Serial.println("NOT FOUND");
    request->send(404, "text/plain", "Not found");
}

String file2str(File file)
{
    String ret;
    ret += "\"";
    ret += file.name();
    ret += "-";
    ret += file.size();
    ret += "\"";

    return ret;
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
            ret += file2str(file);
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

    readPinConfig();
    for (int i = 0; i < PIN_NUMBER; i++)
    {
        pinMode(pins[i], pinsMode[i]);
    }

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

    // for (int i = 0; i < PIN_NUMBER; i++)
    // {
    //     if (pinsEnable[i] && pinsOnenetEnable[i])
    //     {
    //         client[i] = PubSubClient(espClient[i]);
    //         client[i].setServer(mqtt_server, mqtt_port);
    //         mqttConnect(i);
    //     }
    // }

    Serial.print("\nIP Address: ");
    Serial.println(WiFi.localIP());

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

                        Serial.print("Save API ... ");
                        Serial.println(jsonPath);

                        saveJson(apiJson.c_str(), jsonPath.c_str());
                        request->send(200, "text/plain", jsonPath); });
    server.on("/api/load", HTTP_GET, [](AsyncWebServerRequest *request)
              {
            String ret = listDirectory("/storage/api/");
            request->send(200, "text/plain", ret); });
    server.on(
        "/pinconfig/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
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
                saveJson(body_buf.c_str(), configPath.c_str());
                request->send(200, "text/plain", configPath);

                // ESP.restart();
            } });

    server.onNotFound(notFound);
    server.begin();
}

unsigned long previousMillis = 0;
const unsigned long interval = 10000; // 10 seconds
void loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
        Serial.println("Loop");

        // for (int i = 0; i < PIN_NUMBER; i++)
        // {
        //     if (pinsEnable[i] && pinsOnenetEnable[i])
        //     {
        //         if (!client[i].connected()) // If client does not connect to Onenet reconnect
        //         {
        //             mqttConnect(i);
        //         }
        //         client[i].loop();
        //         sendPinValue(i);
        //     }
        // }

        previousMillis = currentMillis;
    }
}
