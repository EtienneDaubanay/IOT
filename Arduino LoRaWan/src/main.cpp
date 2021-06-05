#include <Arduino.h>
#include <string>  
#include <DHT.h>
#include <DHT_U.h>
#include <MKRWAN.h>

LoRaModem modem;

/****Déclaration des identifiant de connexion à The Think network****/

String appEui = "YOUR_API_EUI";
String appKey = "YOUR_APP_KEY";

/****Déclaration des fonctions****/

void SendInt (int value){
  int err;

  modem.beginPacket();

  modem.print(value);

  err = modem.endPacket(true);

  if (err > 0) {

    Serial.println("Message sent correctly!");

  } else {

    Serial.println("Error sending message :");
    Serial.println(err);

  }
}

void SendFloat (float value){
  int err;

  modem.beginPacket();

  modem.print(value);

  err = modem.endPacket(true);

  if (err > 0) {

    Serial.println("Message sent correctly!");

  } else {

    Serial.println("Error sending message :");
    Serial.println(err);

  }
}


/****Déclaration du setup****/

void setup() {

  Serial.begin(115200);

  while (!Serial);

  // change this to your regional band (eg. US915, AS923, ...)

  if (!modem.begin(EU868)) {

    Serial.println("Failed to start module");

    while (1) {}

  };

  bool verbose = false;

  if (verbose){
    Serial.print("Your module version is: ");
    Serial.println(modem.version());
    Serial.print("Your device EUI is: ");
    Serial.println(modem.deviceEUI());
  }
  
  int connected = modem.joinOTAA(appEui, appKey);

  if (!connected) {

    Serial.println("Something went wrong; are you indoor? Move near a window and retry");

    while (1) {}

  }
  else{
    Serial.println("You are connected !\n");
  }

  modem.minPollInterval(60);

}

/****Déclaration du loop****/

void loop() {


  //Mesure de lumiere.
  int luminosite = analogRead(A0);
  Serial.print("Luminosité : ");
  Serial.println(luminosite);

  
  
  // Mesure de temperature.
  float temperature = (analogRead(A1) - 153) * (3.255 / 10);
  Serial.print("Temperature : ");
  Serial.println(temperature);

  
  /**** Envoi des mesures ***/
 
  Serial.println();
  String msg = "";

  msg = "luminosite";
  Serial.print("Sending: " + msg + " - ");
  SendInt(luminosite);
  delay(6000);
  msg = "temperature";
  Serial.print("Sending: " + msg + " - ");
  SendFloat(temperature);

  delay(6000);
  Serial.println();
}
