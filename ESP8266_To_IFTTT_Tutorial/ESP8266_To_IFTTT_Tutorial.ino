//Source: https://github.com/mylob/ESP-To-IFTTT
#include <ESP8266WiFi.h>
#include <arduino.h>
#include "DataToMaker.h"

#define SERIAL_DEBUG // Uncomment this to dissable serial debugging

// define gpio pins here:
#define NUMBER_OF_SENSORS 2 // THIS MUST MATCH THE NUMBER OF SENSORS IN THE SENSOR ARRAY / NO MORE THAN 3

#define FRONT_DOOR_PIN  12  // GPIO12
#define GARAGE_DOOR_PIN 4   // GPIO4
// pin for heatbeat LED
#define HEARTBEAT_PIN   14  // GPIO14

#define PROGAMMING_LED  2   // A tiny blue LED next to the WiFi antenna, connected to GPIO#2 (LOW-->On)
#define SCK_LED         14  // An LED connected to SCK/GPIO#14 (for WeMos D1)

// Define program constants

const char* myKey = ""; // your maker key here; the string after https://maker.ifttt.com/use/
const char* wifissid = "";  // your wifi ssid here
const char* wifipass = ""; // your router password here;  // if no SSL, just leave this blank string

// define program values
int sensors[NUMBER_OF_SENSORS] = {FRONT_DOOR_PIN, GARAGE_DOOR_PIN}; // place your defined sensors in the curly braces
String doorStates[2] = {"Closed", "Open"}; // You can change the LOW / HIGH state strings here

//============================================================================
// CREATE a new maker EVENT with the specified name
DataToMaker event(myKey, "ESPMAIL"); // THIS IS EVENT NAME; You can change it!
// When you create an applet, this EVENT name must match with IFTTT!!!
//============================================================================

// LEAVE SET
int pvsValues[NUMBER_OF_SENSORS];
bool connectedToWiFI = false;

void setup()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  delay(200);
  Serial.println();
#endif

  delay(10); // short delay
  pinMode(HEARTBEAT_PIN, OUTPUT);
  for (int i = 0 ; i < NUMBER_OF_SENSORS ; i++)
  {
    pinMode(sensors[i], INPUT);
    pvsValues[i] = -1; // initialise previous values to -1 to force initial output
  }
  WiFi.mode(WIFI_STA);
  ConnectWifi();
}

void loop() {
  if (wifiConnected)
  {
    if (DetectChange())
    {
      debugln("connecting...");
      if (event.connect())
      {
        debugln("Connected To Maker, Now Trying to Post");
        event.post();
      }
      else debugln("Failed To Connect To Maker!");
    }
    delay(1000); // pause for 1 second
    digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));
  }
  else
  {
    delay(10 * 1000); // 10 seconds delay before trying to re connect
    ConnectWifi();
  }
}

bool ConnectWifi()
{
  // Connect to WiFi network
  debugln();
  debugln();
  debug("Connecting to ");
  debugln(wifissid);
  unsigned long startTime = millis();
  WiFi.begin(wifissid, wifipass);

  while (WiFi.status() != WL_CONNECTED && startTime + 30 * 1000 >= millis()) {
    delay(500);
    debug(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    debugln("");
    debugln("WiFi connected");
  }
  else
  {
    WiFi.disconnect();
    debugln("");
    debugln("WiFi Timed Out!");
  }
}

bool wifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}


bool DetectChange()
{
  int val;
  bool changed = false;
  for (int i = 0 ; i < NUMBER_OF_SENSORS ; i++)
  {
    if ((val = digitalRead(sensors[i])) != pvsValues[i])
    {
      pvsValues[i] = val;
      event.setValue(i + 1, doorStates[val]);
      debug("Changes Detected On Value");
      debugln(String(i + 1));
      changed = true;
    }
  }
  if (!changed) debugln("No Changes Detected");
  return changed;
}


void debug(String message)
{
#ifdef SERIAL_DEBUG
  Serial.print(message);
#endif
}

void debugln(String message)
{
#ifdef SERIAL_DEBUG
  Serial.println(message);
#endif
}

void debugln()
{
#ifdef SERIAL_DEBUG
  Serial.println();
#endif
}


