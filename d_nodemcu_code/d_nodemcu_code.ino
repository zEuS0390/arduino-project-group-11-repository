#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* WIFI_NETWORK_DEFAULT_SSID = "krx";
const char* WIFI_NETWORK_DEFAULT_PASSWORD = "taigamxmn";

const char* mqtt_server = "group11.zeusjames.com";
const char* mqtt_username = "user";
const char* mqtt_password = "pass123";

WiFiClient espClient;
PubSubClient client(espClient); 

void connectToWiFiNetwork(const char* ssid, const char* password, unsigned long timeoutMillis) {
  // Your existing code for connecting to WiFi network
  unsigned long startTime = millis(); // Get the start time
  
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  StaticJsonDocument<400> wifiDoc;
  wifiDoc["title"] = "wifi connect";
  wifiDoc["msg"] = ssid;
  String jsonWifiDoc;
  serializeJson(wifiDoc, jsonWifiDoc);
  Serial.println(jsonWifiDoc);

  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeoutMillis) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    randomSeed(micros());
    StaticJsonDocument<400> wifiSuccessDoc;
    wifiSuccessDoc["title"] = "wifi success";
    wifiSuccessDoc["msg"] = "connected";
    String wifiSuccessStr;
    serializeJson(wifiSuccessDoc, wifiSuccessStr);
    Serial.println(wifiSuccessStr);
  } else {
    StaticJsonDocument<400> wifiErrorDoc;
    wifiErrorDoc["title"] = "wifi error";
    wifiErrorDoc["msg"] = "timeout reached";
    String wifiErrorStr;
    serializeJson(wifiErrorDoc, wifiErrorStr);
    Serial.println(wifiErrorStr);
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    StaticJsonDocument<400> mqttDoc;
    mqttDoc["title"] = "mqtt connect";
    mqttDoc["msg"] = mqtt_server;
    String jsonMqttDoc;
    serializeJson(mqttDoc, jsonMqttDoc);
    Serial.println(jsonMqttDoc);
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      StaticJsonDocument<400> mqttSuccessDoc;
      mqttSuccessDoc["title"] = "mqtt success";
      mqttSuccessDoc["msg"] = "connected";
      String mqttSuccessStr;
      serializeJson(mqttSuccessDoc, mqttSuccessStr);
      Serial.println(mqttSuccessStr);
    } else {
      // Wait 5 seconds before retrying
      StaticJsonDocument<400> mqttFailedDoc;
      mqttFailedDoc["title"] = "mqtt failed";
      mqttFailedDoc["msg"] = "retrying in 5s";
      String mqttSuccessStr;
      serializeJson(mqttFailedDoc, mqttSuccessStr);
      Serial.println(mqttSuccessStr);
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  connectToWiFiNetwork(WIFI_NETWORK_DEFAULT_SSID, WIFI_NETWORK_DEFAULT_PASSWORD, 10000);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnect();
  }
  while (Serial.available() > 0) {
    String jsonStr = Serial.readStringUntil('\n');
    StaticJsonDocument<800> mainDoc;
    DeserializationError error = deserializeJson(mainDoc, jsonStr);
    if (!error) {
      if (mainDoc.containsKey("wifi")) {

        if (mainDoc["wifi"].containsKey("ssid") && mainDoc["wifi"].containsKey("password")) {
          client.disconnect();
          WiFi.disconnect();
          const char* ssid = mainDoc["wifi"]["ssid"];
          const char* password = mainDoc["wifi"]["password"];
          connectToWiFiNetwork(ssid, password, 10000);
        }
        
      }
      client.publish("group11", jsonStr.c_str());
    } else {
      StaticJsonDocument<400> errorDoc;
      errorDoc["title"] = "received json error";
      errorDoc["msg"] = error.c_str();
      String jsonErrorStr;
      serializeJson(errorDoc, jsonErrorStr);
      Serial.println(jsonErrorStr);
    }
  }
  delay(1000);
  client.loop();
}
