#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* WiFi  */
const char* ssid = "YJPhone";  // WiFi SSID
const char* password = "Yangjing";  // WiFi pass

/* MQTT  */
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* temperature_topic = "temperature/data";  
const char* frequency_topic = "temperature/frequency";  


WiFiClient espClient;
PubSubClient client(espClient);

/* port setup */
#define GREEN_LED 18  // connection normal
#define RED_LED 19  // connection abnormal
#define TEMP_SENSOR_PIN 34  

unsigned long lastMsgTime = 0;
unsigned long samplingInterval = 5000;  // sample freq

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

/* MQTT callback */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received in topic: ");
  Serial.println(topic);

  // decode JSON
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Received JSON: " + message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  if (String(topic) == frequency_topic) {
    int newFrequency = doc["frequency"];
    if (newFrequency >= 1000 && newFrequency <= 5000) {
      samplingInterval = newFrequency;
      Serial.print("Updated sampling interval: ");
      Serial.print(samplingInterval);
      Serial.println(" ms");
    }
  }

}

/* MQTT reconnect */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Connected.");
      client.subscribe(frequency_topic);
    } else {
      Serial.println("Failed, retrying...");
      delay(5000);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // LED WiFi status
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }

  unsigned long now = millis();
  if (now - lastMsgTime > samplingInterval) {
    lastMsgTime = now;
    float temperature = analogRead(TEMP_SENSOR_PIN) * (3.3 / 4095.0) * 100; 
    //temperature=25;
    char msg[50];
    snprintf(msg, sizeof(msg), "{\"temperature\": %.2f}", temperature);
    Serial.println("Publishing temperature: " + String(temperature) + " °C");

    client.publish(temperature_topic, msg);
  }
}
