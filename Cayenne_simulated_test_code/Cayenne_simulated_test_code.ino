//#define CAYENNE_DEBUG         // Cayenne Code Uncomment to show debug messages
#define CAYENNE_PRINT Serial  // Cayenne Code Comment this out to disable prints and save space
#include "CayenneDefines.h"
#include <ESP8266WiFi.h>
#include "BlynkSimpleEsp8266.h"
#include "CayenneWiFiClient.h"

#define five_seconds    5000
#define ten_seconds    10000
#define twenty_seconds 20000
#define thirty_seconds 30000
#define one_minute     60000
#define two_minutes   120000
#define three_minutes 180000
#define five_minutes  300000
#define ten_minutes   600000
#define two_hours    7200000

// Cayenne authentication token. This should be obtained from the Cayenne Dashboard.
char token[] = "your_token";
// Your network name and password.
char ssid[] = "your_ssid";
char password[] = "yours";

// Digital Pin
#define SW_PIN            15   // GPIO Pin with Switch connected; for WeMos D1, a 10K pull-down is connected
#define PROGAMMING_LED     2   // A tiny blue LED next to the WiFi antenna, connected to GPIO#2 (LOW-->On)
#define SCK_LED           14   // An LED connected to SCK/GPIO#14 (for WeMos D1)

void setup() {
  // initialize digital pin 2 as an output.
  pinMode(SW_PIN, INPUT);
  pinMode(SCK_LED, OUTPUT);
  pinMode(PROGAMMING_LED, OUTPUT);

  Serial.begin(115200);
  Serial.println("Cayenne simulated test code!");
  Cayenne.begin(token, ssid, password);
  
  Cayenne.run();

  delay(2000);  // wait till WiFi and Cayenne connected
}

float newTempValue = 10.0;    // simulated value
float newHumidValue = 10.0;   // simulated value
float newCelcius = 10.0;      // simulated value
bool  newDigitalValue;        // Real input pin
unsigned int brightness = 0;  // LDR/CDS/photoresistor
unsigned int SW_input;
bool Power_on = true;
bool toggle_LED = false;

unsigned long time_sense = 0;
unsigned long time_cds_sense = 0;

byte error_count = 0;

void loop() {
  Cayenne.run();
  
  if ((millis() > (time_cds_sense + five_seconds)) or (Power_on == true)){
      time_cds_sense = millis();
      // Acquire sensor values, you need to modify this snippet for real sensing
      // Currently values are simulated
      newTempValue += 0.5;   //
      if (newTempValue > 100) newTempValue = 10.0;
      newCelcius = ((newTempValue - 32) * 5 ) / 9;
      newHumidValue += 0.8;  //
      if (newHumidValue > 100) newHumidValue = 10.0;
      Serial.print("Temperature: ");
      Serial.print(newTempValue);
      Serial.print("       Humidity: ");
      Serial.print(newHumidValue);

      brightness = analogRead(A0);
      Serial.print("       Luminocity level: ");
      Serial.print(brightness);

      SW_input = digitalRead(SW_PIN);
      Serial.print("       Switch state: ");
      Serial.println(SW_input);


      if (toggle_LED == false) {
        toggle_LED = true;
        digitalWrite(SCK_LED, HIGH);   // Pilot LED ON
      }
      else {
        toggle_LED = false;
        digitalWrite(SCK_LED, LOW);    // Pilot LED OFF  
      }
  }

  Power_on = false;
}

// This function will be called every time a Dashboard widget writes a value to Virtual Pin 2.
CAYENNE_IN(V2)
{
  CAYENNE_LOG("Got a value: %s", getValue.asStr());
  int i = getValue.asInt();
  
  if (i == 0)
  {
    digitalWrite(PROGAMMING_LED, HIGH);
  }
  else
  {
    digitalWrite(PROGAMMING_LED, LOW);
  }  
}

// These functions are called when the Cayenne widget requests data for the Virtual Pin.
CAYENNE_OUT(V0)
{
  Cayenne.virtualWrite(V0, newTempValue);  // Fahrenheit
}

CAYENNE_OUT(V1)
{
  Cayenne.virtualWrite(V1, newHumidValue); //
}

CAYENNE_OUT(V2)
{
  Cayenne.celsiusWrite(V2, newCelcius);   // Celcius, V2 is already used for control, then this may not work.
}

CAYENNE_OUT(V3)
{
  Cayenne.virtualWrite(V3, brightness);    // LDR/CDS/photoresistor
}

CAYENNE_OUT(V4)
{
  Cayenne.virtualWrite(V4, SW_input);      // 
}
