// Libraries for temp and humidity
#include <DHT.h>
#include <DHT_U.h>

// MQTT Library
#include <PubSubClient.h>


// Pins for temp and humidity
#define DHTPIN 5
#define DHTTYPE DHT11

// Library for wifi
#include <ESP8266WiFi.h>

// Library JSON
#include <ArduinoJson.h>

// Variables for wifi
const char* ssid = "YOUR SSID HERE";
const char* password= "YOUR PASSWORD HERE";

// Variables for misture sensor
const short MOISTURE_THRESHOLD = 10; 
const short DRY_SOIL   = 300 + MOISTURE_THRESHOLD;
const short HUMID_SOIL = 750 + MOISTURE_THRESHOLD;

//Variables for MQTT
#define MQTT_HOST "YOUR HOST HERE"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "YOUR DEVICE ID HERE"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "YOUR TOKEN HERE"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/update/fmt/json"

WiFiClient espClient;
PubSubClient client(espClient);


DHT_Unified dht(DHTPIN, DHTTYPE);

#define LightPIN 4
#define LAMP 0
#define WATER 2
#define MOTION 14
#define Pi 12

void setup() {
  Serial.begin(9600);
  pinMode(LightPIN, INPUT);
  pinMode(LAMP, OUTPUT);
  pinMode(WATER, OUTPUT);
  pinMode(MOTION, INPUT);
  pinMode(Pi, OUTPUT);
  
  dht.begin();
  connectWifi();
  
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(callback);
}

void loop() {  
  //Mqtt loop
  mqttloop();
  Serial.println();
  delay(1000);
}


void connectWifi(){
    delay(10);
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    
    Serial.println("");
    Serial.print("wifi connected with IP adress: ");
    Serial.println(WiFi.localIP());
}


float getTemperature(sensors_event_t event) {
  dht.temperature().getEvent(&event);

  return event.temperature;
}


float getHumidity(sensors_event_t event) {
  dht.humidity().getEvent(&event);

  return event.relative_humidity;
}


String getMoisture() {
  int moisture = analogRead(A0);
  moisture     = map(moisture, 0, 1023, 1023, 0);
  
  if (moisture <= DRY_SOIL) {
    return "Dry soil";
  } else if (moisture <= HUMID_SOIL) {
    return "Humid soil";
  } else {
    return "In Water";
  }
}

boolean getLight() {
   return !digitalRead(LightPIN);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_DISPLAY);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void mqttloop() {

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  sensors_event_t event;

  float temp =  getTemperature(event);
  float humidity = getHumidity(event);
  String moisture = getMoisture();
  boolean light = getLight();
  boolean motion = digitalRead(MOTION);

  String payload = "{\"data\":{\"temp\": " + String(temp,2) + ", \"air\": " + String(humidity,1) + ", \"earth\" : \"" +  moisture + "\" , \"light\" : " + light + " , \"motion\" : " + motion + "}}";

  
  if (client.publish(MQTT_TOPIC, (char*) payload.c_str())) {
    Serial.println(payload);
  }else
  {
    Serial.println("publish failed");
  }
  
  delay(500);

}


void reactToLight(boolean light) {
  if(!light) {
    digitalWrite(LAMP, HIGH);
  }

  else {
    digitalWrite(LAMP, LOW);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Topic = ");
  Serial.print(topic);
  
  Serial.println();
  
  Serial.println("Payload = ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  
  StaticJsonDocument<256> jsonBuffer;
  deserializeJson(jsonBuffer, payload);

  JsonObject root = jsonBuffer.as<JsonObject>();

  if(root["water"] != 0) {
    Serial.println("Watering plants");
    openValve(root["water"]);
  }

  if(root["light"] == 1) {
    Serial.println("Turning light on");
    reactToLight(false);
  }

  
  if(root["picture"] == 1) {
    Serial.println("Taking picture");
    snapPicture();
  }

  if(root["light"] == 0 && root["water"] == 0 && root["picture"] == 0) {
    Serial.println("Turning light off");
    reactToLight(true);
  }
}

void openValve(float openTime) {
  digitalWrite(WATER, HIGH);
  Serial.println("Opened valve");
  
  delay(openTime);

  digitalWrite(WATER, LOW);
  Serial.println("Closed valve");
}

void snapPicture() {
  Serial.println("Sending signal to PI");

  digitalWrite(Pi, HIGH);
  delay(1000);
  digitalWrite(Pi, LOW);
}
