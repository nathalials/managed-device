/*
   Hardware: NodeMCU AMICA v1.0 + NodeMCU Motor Shield + cooler fan + led
   https://www.filipeflop.com/produto/motor-shield-para-modulo-wifi-esp8266-nodemcu/
  

   References IBM Cloud e Watson IoT Platform
   https://developer.ibm.com/recipes/tutorials/run-an-esp8266arduino-as-a-iot-foundation-managed-device/
   https://console.bluemix.net/docs/services/IoT/devices/device_mgmt/index.html
*/

#include <ESP8266WiFi.h> //http://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

//******** Config WiFi
const char* ssid = "";
const char* password = "";

//******** Config Watson IoT Platform
#define ORG ""
#define DEVICE_TYPE ""
#define DEVICE_ID ""
#define TOKEN ""

//-------- Config WIoTP --------
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char publishTopic[] = "iot-2/evt/status/fmt/json";
const char responseTopic[] = "iotdm-1/response";
const char manageTopic[] = "iotdevice-1/mgmt/manage";
const char updateTopic[] = "iotdm-1/device/update";
const char rebootTopic[] = "iotdm-1/mgmt/initiate/device/reboot";

// use the '+' wildcard so it subscribes to any command with any message format
const char commandTopic[] = "iot-2/cmd/+/fmt/+";

// LED - GPIO 12 = D6 on ESP-12E NodeMCU board
const int led = 12;
int publishInterval = 30000; // 30 seconds
long lastPublishMillis;

void callback(char* topic, byte* payload, unsigned int payloadLength);

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void setup() {
  Serial.begin(115200);
  Serial.println();

  wifiConnect();
  mqttConnect();
  initManagedDevice();

  pinMode(5, OUTPUT); // 1,2EN aka D1 pwm left
  pinMode(4, OUTPUT); // 3,4EN aka D2 pwm right

  pinMode(0, OUTPUT); // 1A,2A aka D3
  pinMode(2, OUTPUT); // 3A,4A aka D4

  pinMode(led, OUTPUT);
  digitalWrite (led, LOW);
}

void loop() {
  if (millis() - lastPublishMillis > publishInterval) {
    publishData();
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
    initManagedDevice();
  }
}

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!!!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe("iotdm-1/response")) {
    Serial.println("subscribe to responses OK");
  } else {
    Serial.println("subscribe to responses FAILED");
  }

  if (client.subscribe(rebootTopic)) {
    Serial.println("subscribe to reboot OK");
  } else {
    Serial.println("subscribe to reboot FAILED");
  }

  if (client.subscribe("iotdm-1/device/update")) {
    Serial.println("subscribe to update OK");
  } else {
    Serial.println("subscribe to update FAILED");
  }

  if (client.subscribe("iot-2/cmd/+/fmt/+")) {
    Serial.println("subscribe to command OK");
  } else {
    Serial.println("subscribe to update FAILED");
  }

  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& d = root.createNestedObject("d");
  JsonObject& metadata = d.createNestedObject("metadata");
  metadata["publishInterval"] = publishInterval;
  JsonObject& supports = d.createNestedObject("supports");
  supports["deviceActions"] = true;

  char buff[300];
  root.printTo(buff, sizeof(buff));
  Serial.println("publishing device metadata:"); Serial.println(buff);
  if (client.publish(manageTopic, buff)) {
    Serial.println("device Publish ok");
  } else {
    Serial.print("device Publish failed:");
  }
}

void publishData() {
  String payload = "{\"d\":{\"counter\":";
  payload += millis() / 1000;
  payload += "}}";

  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to.
void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("callback invoked for topic: ");
  Serial.println(topic);

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < payloadLength; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  if (strcmp (responseTopic, topic) == 0) {
    return; // just print of response for now
  }

  if (strcmp (rebootTopic, topic) == 0) {
    Serial.println("Rebooting...");
    ESP.restart();
  }

  if (strcmp (updateTopic, topic) == 0) {
    handleUpdate(payload);
  }

  if (strstr(topic, "led") != NULL) {
    if (messageTemp == "true") {
      digitalWrite(led, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "{msg.payload}") {
      digitalWrite(led, LOW);
      Serial.print("Off");
    }
  }

  if (strstr(topic, "motor") != NULL) {

    if (messageTemp == "true") {
      motorOn();
      Serial.print("Motor On");
    }
    else if (messageTemp == "{msg.payload}") {
      motorOff();
      Serial.print("Motor Off");
    }
  }


}

void handleUpdate(byte* payload) {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char*)payload);
  if (!root.success()) {
    Serial.println("handleUpdate: payload parse FAILED");
    return;
  }
  Serial.println("handleUpdate payload:");
  root.prettyPrintTo(Serial);
  Serial.println();

  JsonObject& d = root["d"];
  JsonArray& fields = d["fields"];
  for (JsonArray::iterator it = fields.begin(); it != fields.end(); ++it) {
    JsonObject& field = *it;
    const char* fieldName = field["field"];
    if (strcmp (fieldName, "metadata") == 0) {
      JsonObject& fieldValue = field["value"];
      if (fieldValue.containsKey("publishInterval")) {
        publishInterval = fieldValue["publishInterval"];
        Serial.print("publishInterval:"); Serial.println(publishInterval);
      }
    }
  }
}

void motorOff(void)
{
  analogWrite(5, 0);
  analogWrite(4, 0);
}
void motorOn(void)
{
  analogWrite(5, 1023);
  analogWrite(4, 1023);
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
}
