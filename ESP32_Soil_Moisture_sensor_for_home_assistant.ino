#define AOUT_PIN 27
//Fins Spider Plant Big
#define Fins_Dry_Spider_plant 3900 //start at 3900
#define Fins_Wet_Spider_plant 3700 //start at 3700
#define Device_Name "Fins_Room_Soil_moisture_Sensor"

#include <config.h> //file containing all config info

#include <WiFi.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>


const char PUB_TOPIC[] = "Fins_Room_Soil_moisture_Sensor_Percent/loopback";
const char SUB_TOPIC[] = "Fins_Room_Soil_moisture_Sensor_Percent/loopback";

WiFiClient network;
MQTTClient mqtt =MQTTClient(256);

unsigned long lastPubTime=0;
void setup() {
  // put your setup code here, to run once:
  delay(4000);
  Serial.begin(9600);
  analogSetAttenuation(ADC_11db);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting");
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("Connected to Wifi");
  connectToMQTT();
}

void loop() {
  int total=0;
  int loops=100;

  for(int x=0;x<loops;x++){
    int value =analogRead(AOUT_PIN);
    total=total+value;
    delay(500);
  }
  int average=total/loops;
  if (average>Fins_Dry_Spider_plant){
    Serial.println("the Plant Is thirsty");
  }
  else if (average<Fins_Wet_Spider_plant){
    Serial.println("Im drowning");
  }
  else{
    Serial.println("Happy Plant :D");
  };
  int percentage = map(average, Fins_Wet_Spider_plant, Fins_Dry_Spider_plant, 100, 0);
  Serial.println ("the Percentage is ");
  Serial.print(percentage);
  Serial.println();
  Serial.print("the Average is: ");
  Serial.println(average);
  sendToMQTT(average);

}

void connectToMQTT(){
  mqtt.begin(MQTT_BROKER_ADDRESS, MQTT_PORT,network);
  mqtt.onMessage(messageHandler);
  Serial.print("Tappign into the hive mind");
  while(!mqtt.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println(mqtt.lastError());
    delay(500);
  }
  Serial.println();
   if (!mqtt.connected()) {
    Serial.println("ESP32 - MQTT broker Timeout!");
    return;
  }
    // Subscribe to a topic, the incoming messages are processed by messageHandler() function
  if (mqtt.subscribe(SUB_TOPIC))
    Serial.print("ESP32 - Subscribed to the topic: ");
  else
    Serial.print("ESP32 - Failed to subscribe to the topic: ");

  Serial.println(SUB_TOPIC);
  Serial.println("ESP32  - MQTT broker Connected!");
}

void sendToMQTT(int data) {
  StaticJsonDocument<200> message;
  message["moisture"] = data;
  char messageBuffer[512];
  serializeJson(message, messageBuffer);

  mqtt.publish(PUB_TOPIC, messageBuffer);

  Serial.println("ESP32 - sent to MQTT:");
  Serial.print("- topic: ");
  Serial.println(PUB_TOPIC);
  Serial.print("- payload:");
  Serial.println(messageBuffer);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("ESP32 - received from MQTT:");
  Serial.println("- topic: " + topic);
  Serial.println("- payload:");
  Serial.println(payload);
} 
