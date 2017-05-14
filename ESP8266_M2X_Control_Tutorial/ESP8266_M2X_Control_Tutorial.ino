#include <SPI.h>
#include <ESP8266WiFi.h>
#include "aJSON.h"
#include "stringbuffer.h"

#define five_seconds    5000
#define ten_seconds    10000
#define one_minute     60000
#define two_minutes   120000
#define three_minutes 180000
#define five_minutes  300000
#define ten_minutes   600000
#define one_hour     3600000
#define two_hours    7200000

#define ESP8266_PLATFORM
//#define M2X_ENABLE_READER // M2X ESP88266 Callback (Fetch) has not been successfully implemented yet. See their github site.

#include <jsonlite.h>
#include <M2XStreamClient.h>

char ssid[] = "yourssid"; //  your network SSID (name)
char pass[] = "yourpass"; // your network password (use for WPA, or use as key for WEP)
//int keyIndex = 0;         // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
bool toggle_LED = false;

char deviceId[] = ""; // Device you want to push to
char updateStreamName[] = "esp1"; // Stream you want to push to
char controlStreamName[] = "control"; // Stream you want to push to
char m2xKey[] = ""; // Your M2X access key (Your PRIMARY API KEY)
char m2xServer[] = "api-m2x.att.com";
char content_type_app_json[] = "Content-Type: application/json";
char content_type_text_html[] = "Content-Type: text/html";
char content_type_text_json[] = "Content-Type: text/json";

unsigned long time_retrieve_interval = 0;
unsigned long time_update_interval = 0;

// Arduino PIN numbers
#define PROGAMMING_LED    2   // A tiny blue LED next to the WiFi antenna, connected to GPIO#2 (LOW-->On)
#define SCK_LED          14   // An LED connected to SCK/GPIO#14 (for WeMos D1)

// Parameters
#define POLL_DELAY_MS   1000 // Poll M2X stream every 1 second
#define FROM_TIME       "1970-01-01T00:00:00.000Z"

WiFiClient client1;
M2XStreamClient m2xClient(&client1, m2xKey);

void setup() {
  Serial.begin(115200);

  pinMode(SCK_LED, OUTPUT);         // pilot LED
  pinMode(PROGAMMING_LED, OUTPUT);  // M2X controlled LED and alert

  while (status != WL_CONNECTED) {
     Serial.print("Attempting to connect to SSID: ");
     Serial.println(ssid);
     // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
     status = WiFi.begin(ssid, pass);

     // wait 5 seconds for connection:
     delay(5000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

#define maximum_wait_time 10000  // wait for 10 seconds at maxmimum and give up
#define json_buffer_size 512     // for aJSON parsing

int response;
unsigned long timeout = 0;
char json_buffer[json_buffer_size];    // for aJSON parsing
word get_iter_count = 1;

void loop() {
  float degreesC, degreesF;
  int response_ptr = 0;                // For decoding JSON
  bool timeout_occurred = false;
  char c;

  if (millis() > (time_update_interval + one_minute)) { // post every minute
     time_update_interval = millis();

     degreesC = random(0,100);       // simulated value to be replaced with real sensor
     degreesF = degreesC * (9.0/5.0) + 32.0;

     Serial.println();
     Serial.print("Degree Celcius: ");
     Serial.print(degreesC);
     Serial.print("  Degree Fahrenheit: ");
     Serial.println(degreesF);

     Serial.println();
     Serial.println("Attempting to update stream values");
     response = m2xClient.updateStreamValue(deviceId, updateStreamName, degreesC);
     Serial.print("M2x client response code: ");
     Serial.println(response);
     if (response == -1) {
        Serial.println("Response error occured, so sleeping...");
        while(1) ;
     }
  }

  if (millis() > (time_retrieve_interval + ten_seconds)) {  // poll "control" every 10 seconds
      time_retrieve_interval = millis();
      Serial.println();
      Serial.println("Attempting to fetch stream values");
      
      getStreamValues();  // send GET REST API to M2X server

      // process response
      timeout = millis() + maximum_wait_time;
      timeout_occurred = false;
      delay(1000);
      while (client1.available() == 0) {   // wait for response for maximum_wait_time seconds
          if (millis() > timeout) {
            Serial.println(">>> Client Timeout !");
            timeout_occurred = true;
            break;
          }
          delay(500);
      }

      if (timeout_occurred == false) {
         bool json_detected = false;
         while (client1.available()) {
            c = client1.read();
            if (c == '{') {json_detected = true; break; }  // I used too simple method to detect the beginning of JSON
            Serial.print(c);
         }
         Serial.println("EoM");
         Serial.println();
         if (json_detected == true) {
            json_buffer[0] = '{';
            response_ptr = 1;
            while (client1.available()) {
              c = json_buffer[response_ptr] = client1.read();
              if ((c == '[') or (c == ']')) ; // these brackets caused exception in aJSON function, so remove them.
              else response_ptr++;
            }
            Serial.print("The number of JSON characters received = ");
            Serial.println(response_ptr);
            json_buffer[response_ptr] = 0;  // make it string
            Serial.println("Parsing...");
            Serial.println(json_buffer);

            int myControl = parseJson(json_buffer); // Parse and get my value back

            if (myControl == 1) {
              Serial.println("My Control Command is ON");
              digitalWrite(PROGAMMING_LED, LOW);   // LED ON, negative logic
            }
            else if (myControl == 2) {
              Serial.println("My Control Command is OFF");
              digitalWrite(PROGAMMING_LED, HIGH);  // LED OFF, negative logic
            }
            else Serial.println("Value out of range or problem in parsing the JSON");
            Serial.println();
         }
         else Serial.println("Packet received, bu no JSON detected!");
      }
      if (client1.connected()) client1.stop(); // stop() closes a TCP connection.
  }
  
  delay(500);
  if (toggle_LED == false) {
     toggle_LED = true;
     digitalWrite(SCK_LED, HIGH);   // Pilot LED ON
  }
  else {
     toggle_LED = false;
     digitalWrite(SCK_LED, LOW);    // Pilot LED OFF  
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void getStreamValues(void)
{
    // Attempt to connect to M2X server
    Serial.print("Attempting to connect to M2X server...  ");
    if (client1.connect(m2xServer, 80)) {
       Serial.println("connected!");
    }
    else {
      // if you didn't get a connection to the m2xServer:
      Serial.println();
      Serial.println("The connection could not be established");
      client1.stop();
      return;
    }
    //build the String with the data that you will send through REST calls to ATT M2X server
    client1.print("GET /v2/devices/");
    Serial.print("GET /v2/devices/");
    client1.print(deviceId);
    Serial.print(deviceId);
    client1.print("/streams/");
    Serial.print("/streams/");
    client1.print(controlStreamName);
    Serial.print(controlStreamName);
    client1.println("/values?limit=1 HTTP/1.1");
    Serial.println("/values?limit=1 HTTP/1.1");
//  Header Part
    client1.print("Host: ");
    Serial.print("Host: ");
    client1.println(m2xServer);
    Serial.println(m2xServer);
    client1.print("X-M2X-KEY: ");
    Serial.print("X-M2X-KEY: ");
    client1.println(m2xKey);
    Serial.println(m2xKey);
    //client1.println("Accept: application/json");
    //Serial.println("Accept: application/json");
    client1.println();
    Serial.println();
}

/**
 * Parse the JSON String. Uses aJson library
 * Refer to http://hardwarefun.com/tutorials/parsing-json-in-arduino
 */
int parseJson(char *jsonString) 
{
    int control = 0;
    float received_value;

    aJsonObject* root = aJson.parse(jsonString);

    if (root != NULL) {
        Serial.println("L1 parsed successfully (root)" );
        aJsonObject* values = aJson.getObjectItem(root, "values"); 

        if (values != NULL) {
            Serial.println("L2 parsed successfully (values)" );
            aJsonObject* value = aJson.getObjectItem(values, "value"); 
            //values->value

            if (value != NULL) {
                Serial.print("L3 parsed successfully (value) = " );
                received_value = value->valuefloat;   // In A2X property, there was no Boolean type.
                Serial.println(received_value);
                if (received_value == 1) control = 1; 
                else if (received_value == 0) control = 2;
                else control = 0;
            }
        }
    }
    aJson.deleteItem(root);
    return control;
}

