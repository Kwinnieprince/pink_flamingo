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

// Variables for wifi
const char* ssid = "ucll-projectweek-IoT";
const char* password= "Foo4aiHa";

// Variables for misture sensor
const short MOISTURE_THRESHOLD = 10; 
const short DRY_SOIL   = 300 + MOISTURE_THRESHOLD;
const short HUMID_SOIL = 750 + MOISTURE_THRESHOLD;

//Variables for MQTT
#define MQTT_HOST "d0ftne.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:d0ftne:bart:ESPTempSensor"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "password"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/status/fmt/json"

WiFiClient espClient;
PubSubClient client(espClient);


DHT_Unified dht(DHTPIN, DHTTYPE);

#define LightPIN 4
#define LAMP 0
#define WATERKRAAN 2

void setup() {
  Serial.begin(9600);
  pinMode(LightPIN, INPUT);
  pinMode(LAMP, OUTPUT);
  pinMode(WATERKRAAN, OUTPUT);
  
  dht.begin();
  connectWifi();
  
  client.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {  
  //Mqtt loop
  mqttloop();
  Serial.println();
  delay(5000);
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
      client.subscribe(MQTT_TOPIC_DISPLAY );
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

  handleEvents(temp, humidity, moisture, light);
  
  String payload = "{\"data\":{\"temp\": " + String(temp,2) + ", \"air\": " + String(humidity,1) + ", \"earth\" : \"" +  moisture + "\" , \"light\" : " + light + "}}";

  
  if (client.publish(MQTT_TOPIC, (char*) payload.c_str())) {
    Serial.println(payload);
  }else
  {
    Serial.println("publish failed");
  }
  
  delay(500);

}

void handleEvents(float temp, float humidity, String moisture, boolean light) {
  reactToLight(light);
}

void reactToLight(boolean light) {
  if(!light) {
    digitalWrite(LAMP, HIGH);
    Serial.println("It's dark so i turn on the light");
  }

  else {
    digitalWrite(LAMP, LOW);
    Serial.println("No light needed to i turn off the light");
  }
}
