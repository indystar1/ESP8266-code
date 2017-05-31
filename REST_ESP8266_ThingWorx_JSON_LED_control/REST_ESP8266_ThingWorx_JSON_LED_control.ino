/****************************************************************************
Arduino to Thingworx REST POST and GET using ESP8266

http://academic.cloud.thingworx.com/Thingworx/Things/ESP8266_REST_yours/Services/setTempAndHumid?appKey=your_app_key&method=post&Temp=53.70&Humid=34.50&Bool=true&Analog=123.45 (Content-Type: application/json)

http://academic.cloud.thingworx.com/Thingworx/Things/ESP8266_REST_yours/Properties/MyControl?method=get&appKey=your_app_key (Content-Type: application/json)

This is based on the following link.
https://www.thingworx.com/ecosystem/academic-program/iot-projects/weather-app-arduino-uno/

This sketch reads four (two simulated) sensor values and sends to a Thing on ThingWorx.
It also gets a value from Mashup to control a device such as LED.

On the Thingworx side make sure your thingName[], your serviceName[] and your server[] match exactly.
Use your provided appKey that you receive when you set up your Thing.

Create a service in your Thing or ThingShape and create inputs that will match the variable names you use
in your Arduino code.  In the JavaScript window of Services, type something similar to the following:

var isTrueSet = (Bool.toLowerCase() === 'true');
me.Temperature = parseFloat(Temp) ;
me.Humidity = parseFloat(Humid) ;
me.BoolStatus = isTrueSet ;
me.AnalogValue = parseFloat(Analog) ;

Everything is case sensitive.

It uses Arduino ESP8266 library, aJson code
https://github.com/interactive-matter/aJson/

created 4/3/2016
modified 3/23/2017
modified 5/27/2017
by Indy Star

This code was based on
Arduino to Thingworx Ethernet Web Client using Ethernet Shield
by Nick Milleson
Design Engineer
EAC Product Development Solutions
*****************************************************************************/

#include <SPI.h>
#include <ESP8266WiFi.h>
#include "aJSON.h"
#include "stringbuffer.h"

#define one_second      1000
#define five_seconds    5000
#define ten_seconds    10000
#define twenty_seconds 20000
#define twentyfive_seconds 25000
#define thirty_seconds 30000
#define one_minute     60000
#define two_minutes   120000
#define three_minutes 180000
#define five_minutes  300000
#define ten_minutes   600000
#define two_hours    7200000

#define json_buffer_size 512

const char* wifissid = "wifissid";
const char* wifipass = "wifipassword"; // if no SSL, just leave this blank string
const char* server = "academic.cloud.thingworx.com";

WiFiClient wifiClient;

//ThingWorx App-key which replaces login credentials
char appKey[] = "your_app_key";

// ThingWorx Thing name for which you want to set properties values
char thingNameTH[] = "ESP8266_REST_yours";

// ThingWorx Services name that will set values for the properties
// See the documentation for this tutorial for more information
char serviceNameTH[] = "setTempAndHumid";

//Initialize Input Names Array (this must match with those names in setTempAndHumid service!!!)
char* inputNamesTH[] = {"Temp", "Humid", "Bool", "Analog"};

float newTempValue = 10.0;    // simulated value
float newHumidValue = 10.0;   // simulated value
bool  newDigitalValue;        // Real input pin
int   newAnalogValue = 0;     // connect an analog sensor (such as CDS photoresistor)
char  propertyB1[] = "MyControl"; // this is the property name used to control a device such as an LED
                                 // Mashup will need a control button or checkbox for this
                                 // It will be retrieved by GET method

char content_type_app_json[] = "Content-Type: application/json";
char content_type_text_html[] = "Content-Type: text/html";
char content_type_text_json[] = "Content-Type: text/json";

unsigned long timeout = 0;

#define resp_wait_time 10000  // 10 seconds
unsigned long time_alive_interval = 0;
unsigned long time_get_interval = 0;
unsigned long time_post_interval = 0;

#define PROGAMMING_LED 2 // A tiny blue LED next to the WiFi antenna, connected to GPIO#2 (LOW-->On)
#define SCK_LED     14 // An LED connected to SCK/GPIO#14 (I used WeMos D1)
#define SW1Status   12 // a switch is connected; High if on, Low if off
#define ANALOG_PIN  A0 // for luminosity

boolean TW_connected = false;

void setup() {
  Serial.begin(115200);
  timeout = 0;
  while (!Serial) {
    delay(200);
	timeout++;
	if (timeout > 25) break;  // wait up to 5 seconds
  }

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifissid);

  // Connecting to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(wifissid, wifipass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
 
  printWifiStatus();

  // Now connect to TW server. If you get a connection, report back via serial:
  if (wifiClient.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(server);
    Serial.println();
    TW_connected = true;
  }
  else {
    // if you didn't get a connection to the server:
    Serial.println("The connection could not be established");
    wifiClient.stop();
    TW_connected = false;
    return;
  }

  pinMode(SW1Status, INPUT);
  pinMode(PROGAMMING_LED, OUTPUT);
  pinMode(SCK_LED, OUTPUT);           // Pilot LED, and turn it off
  digitalWrite(PROGAMMING_LED, HIGH); // LED off, negative logic
}

word post_iter_count = 1;
word get_iter_count = 1;
bool toggle_LED = false;
bool power_on = true;
char json_buffer[json_buffer_size];

void loop() {

  int response_ptr = 0;         // For decoding JSON
  bool timeout_occurred = false;
  char c;

  // Toggle pilot LED every 5 seconds
  if ( (millis() > (time_alive_interval + five_seconds)) or (power_on == true) ) {
      time_alive_interval = millis();
      if (toggle_LED == false) {
        toggle_LED = true;
        digitalWrite(SCK_LED, HIGH);   // Pilot LED ON
      }
      else {
        toggle_LED = false;
        digitalWrite(SCK_LED, LOW);    // Pilot LED OFF  
      }
  }

  // Post sensor values every 10 seconds
  if ( (millis() > (time_post_interval + ten_seconds)) or (power_on == true) ) {
      time_post_interval = millis();

      Serial.print("Post_iter_count is ");
      Serial.println(post_iter_count++);   
      // Clear anything in the wifiClient buffer before initiating next POST or GET
      Serial.println("Clearing wifiClient buffer!");
      while (wifiClient.available()) {
        c = wifiClient.read();
        Serial.print(c);
      }
      Serial.println();
      
      // Acquire sensor values, you need to modify this snippet for real sensing
      // Currently values are simulated
      newTempValue += 0.5; //myHumidity.readTemperature();
      if (newTempValue >= 100) newTempValue = 10.0;
      newHumidValue += 0.8; //myHumidity.readHumidity();
      if (newHumidValue >= 100) newHumidValue = 10.0;

      if (digitalRead(SW1Status) == HIGH) newDigitalValue = true; 
      else newDigitalValue = false;
      newAnalogValue = analogRead(ANALOG_PIN); 

      sendValues(); // send the acquired values

      // process response
      timeout = millis() + resp_wait_time;
      timeout_occurred = false;
      delay(1000);
      while (wifiClient.available() == 0) {   // wait for response for resp_wait_time seconds
         if (millis() > timeout) {
            Serial.println(">>> Client Timeout !");
            timeout_occurred = true;
            if (wifiClient.connect(server, 80)) {
              Serial.print("connected to ");
              Serial.println(server);
              Serial.println();
            }
            else {
              // if you didn't get a connection to the server:
              Serial.println("the connection could not be established");
              wifiClient.stop();
            }
            break;
         }
         delay(500);
     }
     if (timeout_occurred == false) {
         while (wifiClient.available()) {
            c = wifiClient.read();
            Serial.print(c);
         }
         Serial.println("EoM");
         Serial.println();
     }
  }

  // Get control value every 10 seconds
  if ( (millis() > (time_get_interval + ten_seconds))  or (power_on == true) ) {
      time_get_interval = millis();
      Serial.print("Get_iter_count is ");
      Serial.println(get_iter_count++);   

      getValues();

      // process response
      timeout = millis() + resp_wait_time;
      timeout_occurred = false;
      delay(1000);
      while (wifiClient.available() == 0) {   // wait for response for resp_wait_time seconds
          if (millis() > timeout) {
            Serial.println(">>> Client Timeout !");
            timeout_occurred = true;
            break;
          }
          delay(500);
      }

      if (timeout_occurred == false) {
         bool json_detected = false;
         while (wifiClient.available()) {
            c = wifiClient.read();
            if (c == '{') {json_detected = true; break; }
            Serial.print(c);
         }
         Serial.println("EoM");
         Serial.println();
         if (json_detected == true) {
            json_buffer[0] = '{';
            response_ptr = 1;
            while (wifiClient.available()) {
              c = json_buffer[response_ptr] = wifiClient.read();
              if ((c == '[') or (c == ']')) ; // these brackets caused exception in aJSON function, so remove them.
              else response_ptr++;
            }
            Serial.print("The number of JSON characters received = ");
            Serial.println(response_ptr);
            json_buffer[response_ptr] = 0;  // make it string
            Serial.println("Parsing...");
            Serial.println(json_buffer);

            int MyControl = parseJson(json_buffer); // Parse and get my value back

            if (MyControl == 1) {
              Serial.println("MyControl is on");
              digitalWrite(PROGAMMING_LED, LOW);   // LED ON, negative logic
            }
            else if (MyControl == 2) {
              Serial.println("MyControl is off");
              digitalWrite(PROGAMMING_LED, HIGH);  // LED OFF, negative logic
            }
            else Serial.println("There was some problem in parsing the JSON");
            Serial.println();
         }
         else Serial.println("Packet received, bu no JSON detected!");
      }
   }
   power_on = false;
}

/**
 * Parse the JSON String. Uses aJson library
 * Refer to http://hardwarefun.com/tutorials/parsing-json-in-arduino
 */
int parseJson(char *jsonString) 
{
    int value = 0;

    aJsonObject* root = aJson.parse(jsonString);

    if (root != NULL) {
        Serial.println("L1 parsed successfully (root)" );
        aJsonObject* rows = aJson.getObjectItem(root, "rows"); 

        if (rows != NULL) {
            Serial.println("L2 parsed successfully (rows)" );
            aJsonObject* MyControl = aJson.getObjectItem(rows, "MyControl"); 
            //rows->name

            if (MyControl != NULL) {
                Serial.println("L3 parsed successfully (MyControl)" );
                if (MyControl->valuebool == true) value = 1; 
                else if (MyControl->valuebool == false) value = 2;
                else value = 0;
            }
        }
    }
    aJson.deleteItem(root);
    return value;
}

char data[160];

void sendValues(void)
{
    //build the String with the data that you will send through REST calls to your TWX server
    strcpy(data, "?appKey=");
    strcat(data, appKey);
    strcat(data, "&method=post");

    // send the HTTP POST request:
    wifiClient.print("POST /Thingworx/Things/");
    wifiClient.print(thingNameTH);
    wifiClient.print("/Services/");
    wifiClient.print(serviceNameTH);
    wifiClient.print(data);
    wifiClient.print("&");
    wifiClient.print(inputNamesTH[0]);
    wifiClient.print("=");
    wifiClient.print(newTempValue);
    wifiClient.print("&");
    wifiClient.print(inputNamesTH[1]);
    wifiClient.print("=");
    wifiClient.print(newHumidValue);
    wifiClient.print("&");
    wifiClient.print(inputNamesTH[2]);
    wifiClient.print("=");
    if (newDigitalValue == true) wifiClient.print("true&");
    else wifiClient.print("false&");
    wifiClient.print(inputNamesTH[3]);
    wifiClient.print("=");
    wifiClient.print(newAnalogValue);
    wifiClient.println(" HTTP/1.1");
    wifiClient.print("Host: ");
    wifiClient.println(server);
    wifiClient.println(content_type_app_json);
    wifiClient.println();

    // print the request out to Serial monitor
    Serial.print("POST /Thingworx/Things/");
    Serial.print(thingNameTH);
    Serial.print("/Services/");
    Serial.print(serviceNameTH);
    Serial.print(data);
    Serial.print("&");
    Serial.print(inputNamesTH[0]);
    Serial.print("=");
    Serial.print(newTempValue);
    Serial.print("&");
    Serial.print(inputNamesTH[1]);
    Serial.print("=");
    Serial.print(newHumidValue);
    Serial.print("&");
    Serial.print(inputNamesTH[2]);
    Serial.print("=");
    if (newDigitalValue == true) Serial.print("true&");
    else Serial.print("false&");
    Serial.print(inputNamesTH[3]);
    Serial.print("=");
    Serial.print(newAnalogValue);
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(server);
    Serial.println(content_type_app_json);
    Serial.println();
}

void getValues(void)
{
    //build the String with the data that you will send through REST calls to your TWX server
    strcpy(data, "?appKey=");
    strcat(data, appKey);
    strcat(data, "&method=get");

    wifiClient.print("GET /Thingworx/Things/");
    wifiClient.print(thingNameTH);
    wifiClient.print("/Properties/");
    wifiClient.print(propertyB1);
    wifiClient.print(data);
    wifiClient.println(" HTTP/1.1");
    wifiClient.println("Accept: application/json");
    wifiClient.print("Host: ");
    wifiClient.println(server);
    wifiClient.println(content_type_text_json);
    wifiClient.println();

    // print the request out to Serial monitor
    Serial.print("GET /Thingworx/Things/");
    Serial.print(thingNameTH);
    Serial.print("/Properties/");
    Serial.print(propertyB1);
    Serial.print(data);
    Serial.println(" HTTP/1.1");
    Serial.println("Accept: application/json");
    Serial.print("Host: ");
    Serial.println(server);
    Serial.println(content_type_text_json);
    Serial.println();
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
