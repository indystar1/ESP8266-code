#include <ESP8266WiFi.h>

#define ESP8266_PLATFORM
#include "M2XStreamClient.h"

char wifissid[] = "your_wifi_ssid"; //  your WiFi SSID (name)
char wifipass[] = "your_wifi_pass"; // your WiFi password (use for WPA, or use as key for WEP)
int keyIndex = 0;         // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
bool toggle_LED = false;

char deviceId[] = ""; // Device you want to push to
char streamName[] = ""; // the stream (not device) name you want to push this data to
char m2xKey[] = ""; // Your PRIMARY API KEY

// Arduino PIN numbers
#define temperaturePin   16 // for real Temperature sensor
#define SCK_LED          14 // An LED connected to SCK/GPIO#14 (for WeMos D1)

WiFiClient client;
M2XStreamClient m2xClient(&client, m2xKey);

void setup() {
  Serial.begin(115200);

  pinMode(temperaturePin, INPUT);
  pinMode(SCK_LED, OUTPUT);

  // Connecting to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(wifissid, wifipass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  printWifiStatus();
}

void loop() {
  float voltage, degreesC, degreesF;

  //degreesC = readTemp(temperaturePin); // enable and modify this if you have real sensor
  degreesC = random(0,100) ; // just simulating temperature
  degreesF = degreesC * (9.0/5.0) + 32.0; // Fahrenheit

  Serial.print("voltage: ");
  Serial.print(voltage);
  Serial.print("  deg C: ");
  Serial.print(degreesC);
  Serial.print("  deg F: ");
  Serial.println(degreesF);

  int response = m2xClient.updateStreamValue(deviceId, streamName, degreesC);
  Serial.print("M2x client response code: ");
  Serial.println(response);

  if (response == -1) while(1); // gave up

  if (toggle_LED == false) {
     toggle_LED = true;
     digitalWrite(SCK_LED, HIGH);   // Pilot LED ON
  }
  else {
     toggle_LED = false;
     digitalWrite(SCK_LED, LOW);    // Pilot LED OFF  
  }

  delay(10000);  // update every 10 seconds
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
