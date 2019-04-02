#include <Adafruit_Sensor.h>
#include "DHT.h"
#define DHTPIN D0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println("----------------");
  Serial.println("DHT Starting");
  Serial.println("----------------");
}
void loop() {
  Serial.print("Temperature = ");
  float temp = dht.readTemperature();
  Serial.println(String(temp, 1));
  delay(2000); 
}
