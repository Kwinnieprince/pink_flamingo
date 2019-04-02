// Libraries for temp and humidity
#include <DHT.h>
#include <DHT_U.h>

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




DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  connectWifi();
}

void loop() {

  sensors_event_t event;

  float temp = getTemperature(event);
  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.println("°C");  
  
  float hum = getHumidity(event);
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  String moist = getMoisture();
  Serial.print("Moisture : ");
  Serial.println(moist);

  Serial.println();
  
  delay(2000); 
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
    return "In Water!";
  }
}

