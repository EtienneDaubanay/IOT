#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/*** WiFi parametres***/
#define WLAN_SSID       "YOUR_VIFI_NAME"
#define WLAN_PASS       "YOUR_WIFI_PASSWORD"

/*** Adafruit IO ***/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "YOUR_ADAFRUIT_USERNAME"
#define AIO_KEY         "YOUR_ADAFRUIT_APP_KEY" 


/*** Parrametrisation de votre MQTT client ***/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Temperature_Etienne = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature_Etienne");
Adafruit_MQTT_Publish Humidity_Etienne = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity_Etienne");

//Declaration de la pression moyenne sur terre.
#define SEALEVELPRESSURE_HPA (1013.25)

/*** Delcaration du bus SPI ***/
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

/*** Déclaration du motion sensor ***/
#define timeSeconds 2
const int motionSensor = 33;
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;

/*** Fonction utilisé par l'interuption ***/
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  startTimer = true;
  lastTrigger = millis();
}

void setup() {
  
  Serial.begin(115200);

  /*** Parametrisation du motion sensor ***/
  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  
  /*** Detection du capteur BME ***/
    if (! bme.begin()) {
  Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
  while (1);

  
}
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
  
  // Connection au Wifi
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // Connection à Adafruit IO.
  connect();
}

  // connection à Adafruit IO via MQTT.
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  Serial.println(F("Adafruit IO reussi!"));
}

void loop() {

  /*** Verification de la cnnexion à Adafruit IO ***/
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }
  
  /*** Détecteur de movement ***/
  now = millis();
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    startTimer = false;
  }
  
  
  /*** Capteur BME5680 ***/
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);
  delay(50); 
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  float temp = bme.temperature;
  Serial.print(temp);
  Serial.println(F(" *C"));

  Serial.print(F("Pressure = "));
  float pressure = bme.pressure / 100.0; 
  Serial.print(pressure);
  Serial.println(F(" hPa"));

  Serial.print(F("Humidity = "));
  float hum = bme.humidity;
  Serial.print(hum);
  Serial.println(F(" %"));

  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));

  Serial.print(F("Approx. Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(F(" m"));

  Serial.println();
  //Envoie toute les 30s.
  delay(3000);

  /*** Envoie à Adafruit ***/
   
   if (! Temperature_Etienne.publish(temp)) {
      Serial.println(F("Failed"));
    } 
       if (! Humidity_Etienne.publish(hum)) {
      Serial.println(F("Failed"));
    }
    else {
      Serial.println(F("Sent!"));
    }
}
