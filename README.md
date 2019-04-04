
# Pink flamingo's automated plant health system

## This readme is not updated anymore

for more information, check the [wiki](https://github.com/Kwinnieprince/pink_flamingo/wiki)


## Introduction

This school project is a projcet to monitor the health of a plant.
The plant will check the humidity of the soil, the amount of light, the humidity of the air and the temperature. There is also a motion sensor. This can be used when someone tries to steal your plant.
We have a solanoid valve that can water your plant automatically.

### parts in this wiki

* Requirements
* Programming the ESP module
* Starting with the IBM cloud
* Using NodeRed

### Requirements

* 1 ESP-8266 (NodeMcu)
* 1 DHT11 or a DHT22 if you want to have more accurate results
* A PID sensor (Motion sensor)
* Soil moisture sensor (Flying fish)
* Light sensor (Flying fish)
* Electric solenoid valve 12V DC Normally closed
* Songle SRD-05VDC-SL-C (Relay)
* Some breadboards (we used 3)
* Tubing for watering the plant (In case you have the solenoid valve, you need to use threaded 1/4inch tubing)
* An LED
* Jumper wires (Male-Male (Homosexual) & Male-Female (Heterosexual) & Female-Female (Lesbian))
* Micro-USB cable
* 12V DC power outlet

### First steps

1. set the ESP module on a breadboard

    <img src="pictures/IMG_7188.jpg" alt="esp module" width="300">

2. Setup the sensors on the breadboard and set the voltage pins correctly

3. Connect the sensors to the ESP module

    <img src="pictures/earthSensor.png" alt="earth sensor" width="300">
    <img src="pictures/airSensor.png" alt="earth sensor" width="300">
    <img src="pictures/Led.png" alt="earth sensor" width="300">
    <img src="pictures/lightSenor.png" alt="earth sensor" width="300">
    <img src="pictures/relay.png" alt="earth sensor" width="300">

4. Now we can start programming the ESP module and use the IBM cloud

### Programming the ESP module

* First, we need to connect to the wifi

```C++
// Library for wifi
#include <ESP8266WiFi.h>

// Variables for wifi
const char* ssid = "Your-SSID-here";
const char* password= "Your-password-here";

void setup() {
    Serial.begin(9600);

    connectWifi();
}

void loop() { 
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
```

* If we are connected succesfully to the wifi, we can set up the MQTT service

> We are going to connect the service to the IBM cloud later in this manual.

``` C++
    // MQTT Library
    #include <PubSubClient.h>

    //Variables for MQTT
    #define MQTT_HOST "Your-host-here"
    #define MQTT_PORT 1883
    #define MQTT_DEVICEID "Your-device-ID-here"
    #define MQTT_USER "use-token-auth"
    #define MQTT_TOKEN "Your-token here"
    #define MQTT_TOPIC "iot-2/evt/status/fmt/json" // you can choose other topics if you dont't want to use the IBM cloud
    #define MQTT_TOPIC_DISPLAY "iot-2/cmd/status/fmt/json" // you can choose other topics if you dont't want to use the IBM cloud

    WiFiClient espClient;
    PubSubClient client(espClient);

    void setup() {
    Serial.begin(9600);

    //For connecting wifi
    connectWifi();

    //For connecting the MQTT service
    client.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {  
    //Mqtt loop
    mqttloop();
    Serial.println();
    delay(5000);
}

void mqttloop() {

    if (!client.connected()) {
        reconnect();
    }

    client.loop();

    sensors_event_t event;

    //These methods will be added later in this guide when we read the code of the sensors
    // float temp =  getTemperature(event);
    // float humidity = getHumidity(event);
    // String moisture = getMoisture();
    // boolean light = getLight();
    //handleEvents(temp, humidity, moisture, light);

    //This will be the payload that will be sent with MQTT
    String payload = "{\"data\":{\"temp\": " + String(temp,2) + ", \"air\": " + String(humidity,1) + ", \"earth\" : \"" +  moisture + "\" , \"light\" : " + light + "}}"; 


    if (client.publish(MQTT_TOPIC, (char*) payload.c_str())) {
        Serial.println(payload);
    }else
    {
        Serial.println("publish failed");
    }

    delay(500);

}
```

* Once the MQTT connection is set up, we can start reading the values of the different sensors.
    * We will start by connecting the humidity sensor
    * If the sensor gives a value between:
        * 0-300 &rarr; then the soil is dry
        * 300-750 &rarr; then the soil is moisturized
        * 750 - ... &rarr; then the plant is in water or has too much water
    * When the connector is connected, we can connect the other sensors

> We chose to use the light sensor on a digital port, this means that the sensor can only say if there is light or not (0, 1). The soil-humidity sensor is connected to the serial port. This means that we can read  multiple values than 0 and 1.

``` C++
  // Libraries for temp and humidity
  #include <DHT.h>
  #include <DHT_U.h>

// Variables for misture sensor
const short MOISTURE_THRESHOLD = 10;
const short DRY_SOIL   = 300 + MOISTURE_THRESHOLD;
const short HUMID_SOIL = 750 + MOISTURE_THRESHOLD;

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

```

### Starting with the IBM cloud

### Using NodeRed