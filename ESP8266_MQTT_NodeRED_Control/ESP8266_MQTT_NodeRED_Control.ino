// This is for WeMos D1 ESP8266 Device

#include <SPI.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define five_seconds    5000
#define ten_seconds    10000
#define one_minute     60000
#define two_minutes   120000
#define three_minutes 180000
#define five_minutes  300000
#define ten_minutes   600000
#define one_hour     3600000
#define two_hours    7200000

const char* wifissid = "your_wifi_ssid";
const char* wifipass = "your_wifi_pass"; // if no SSL, just leave this blank string

const char* topic = "IoTLab1/NodeTest";

#define MQTTserver "192.168.1.2"  // your mqtt broker; you have to change this
#define RETRY_MAXIMUM  10

WiFiClient wifiClient;

PubSubClient mqtt(wifiClient, MQTTserver, 1883);

int debug = 0;

bool toggle_LED = false;
bool power_on = true;
byte ten_min_count = 0;
bool any_fail_flag = false;


#define SW1Status         12   // a switch is connected; for WeMos D1, a pull-down resistor is connected
#define PROGAMMING_LED     2   // A tiny blue LED next to the WiFi antenna, connected to GPIO#2 (LOW-->On)
#define SCK_LED           14   // An LED connected to SCK/GPIO#14 (for WeMos D1)
#define ANALOG_PIN        A0

bool mqtt_connected = false;

unsigned long time_update_interval;
unsigned long time_count_sense;
unsigned long time_alert_sense;
unsigned long time_LED_sense;
unsigned long time_STATUS_sense;
unsigned long time_WiFi_sense;
unsigned long time_callback_sense;

byte error_count = 0;
String msgString;

void setup()
{
  Serial.begin(115200);
  delay(100);

  //gpio_init();

  pinMode(SW1Status, INPUT);
  pinMode(SCK_LED, OUTPUT);
  pinMode(PROGAMMING_LED, OUTPUT);
  digitalWrite(PROGAMMING_LED, LOW); // Warning LED on

  // connecting to a WiFi network
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

  digitalWrite(PROGAMMING_LED, HIGH); // Warning LED off
  time_callback_sense = millis();
}

void(* resetFunc) (void) = 0; // declare reset function at address 0 for soft reset

#define BUFFER_SIZE 100

void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  String payload = pub.payload_string();
  Serial.println(payload);
  byte try_count = 0;

  if (pub.topic() == "IoTLab1/NodeRED/LED_ALERT") // if this is my task
  {
    if (payload == "true")
    {
      digitalWrite(PROGAMMING_LED, LOW);  // On
    }
    else if (payload == "false")
    {
      digitalWrite(PROGAMMING_LED, HIGH); // Off
    }
  }
}

byte retry_count = 0;
float degreesC = 0;
float degreesF = 0;
float humid = 0;
unsigned int brightness = 0;
unsigned int SW_input;

void loop() {
  char message_buff[BUFFER_SIZE];

  retry_count = 0;
  while (!mqtt.connected())  {
    // Connect (or reconnect) to mqtt broker on the openhab server
    Serial.print("Connecting to ");
    Serial.println(MQTTserver);

    if (mqtt.connect("esp8266_NodeTest1")) { // this is ID, so you have to change whenever creating a new node
      Serial.println("Connected to MQTT broker");
      mqtt_connected = true;
      mqtt.set_callback(callback);

      retry_count = 0;
      while (!mqtt.publish(topic, "Hello~ from NodeTest 1")) {
        Serial.println("Publish failed");
        retry_count++;
        if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
        delay(1000);
      }
      Serial.println("Publish ok");

      retry_count = 0;
      while (!mqtt.subscribe("IoTLab1/#")) {
        Serial.println("Subscription failed");
        retry_count++;
        if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
        delay(1000);
      }
      Serial.println("Subscription success");
    }
    else {
      retry_count++;
      mqtt_connected = false;
      Serial.println("MQTT connection failed");
      if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
      delay(1000);
    }
  }

  retry_count = 0;
  while (!mqtt.loop()) {
    ; // to process incoming messages and maintain its connection to the server
    // false:the client is no longer connected; true:the client is still connected
    mqtt_connected = false;
    Serial.println("MQTT disconnected");
    retry_count++;
    if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
    delay(1000);
  }

  if (millis() > (time_update_interval + ten_seconds)) { // post every
    time_update_interval = millis();
    debug++;
    /*dtostrf(debug, 0, 0, message_buff);
    retry_count = 0;
    while (!mqtt.publish("IoTLab1/NodeTest/Count", message_buff)) {
      Serial.println("Publish failed");
      retry_count++;
      if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
      delay(1000);
    }
    Serial.print("Published: IoTLab1/NodeTest/Count ");
    Serial.println(message_buff);*/

    brightness = analogRead(A0);
    Serial.print("Luminocity level is ");
    Serial.println(brightness);
    msgString = String(brightness);
    msgString.toCharArray(message_buff, msgString.length() + 1);
    //Serial.print("About to publish: IoTLab1/NodeTest/Luminocity ");
    //Serial.println(message_buff);
    retry_count = 0;
    while (!mqtt.publish("IoTLab1/NodeTest/Luminocity", message_buff)) {
      Serial.println("Publish failed");
      retry_count++;
      if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
      delay(1000);
    }
    Serial.print("Published: IoTLab1/NodeTest/Luminocity ");
    Serial.println(message_buff);

    degreesC = random(0, 100);      // simulated value to be replaced with real sensor
    degreesF = degreesC * (9.0 / 5.0) + 32.0;

    Serial.println();
    Serial.print("Degree Celcius: ");
    Serial.print(degreesC);
    Serial.print("  Degree Fahrenheit: ");
    Serial.println(degreesF);

    msgString = String(degreesC);
    msgString.toCharArray(message_buff, msgString.length() + 1);
    //Serial.print("About to publish: IoTLab1/NodeTest/Temperature ");
    //Serial.println(message_buff);
    retry_count = 0;
    while (!mqtt.publish("IoTLab1/NodeTest/Temperature", message_buff)) {
      Serial.println("Publish failed");
      retry_count++;
      if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
      delay(1000);
    }
    Serial.print("Published: IoTLab1/NodeTest/Temperature ");
    Serial.println(message_buff);

    /*humid = random(0, 100);      // simulated value to be replaced with real sensor
    msgString = String(humid);
    msgString.toCharArray(message_buff, msgString.length() + 1);
    retry_count = 0;
    while (!mqtt.publish("IoTLab1/NodeTest/Humidity", message_buff)) {
      Serial.println("Publish failed");
      retry_count++;
      if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
      delay(1000);
    }
    Serial.print("Published: IoTLab1/NodeTest/Humidity ");
    Serial.println(message_buff); */

    SW_input = digitalRead(SW1Status);
    Serial.print("Switch state: ");
    Serial.println(SW_input);
    msgString = String(SW_input);
    msgString.toCharArray(message_buff, msgString.length() + 1);
    retry_count = 0;
    while (!mqtt.publish("IoTLab1/NodeTest/SW_input", message_buff)) {
      Serial.println("Publish failed");
      retry_count++;
      if (retry_count > RETRY_MAXIMUM) resetFunc(); // call reset
      delay(1000);
    }
    Serial.print("Published: IoTLab1/NodeTest/SW_input ");
    Serial.println(message_buff);

  }

  //if ((millis() > (time_LED_sense + five_seconds)) or (power_on == true)) {
  time_LED_sense = millis();

  if (toggle_LED == false) {
    toggle_LED = true;
    digitalWrite(SCK_LED, HIGH);   // Pilot LED ON
  }
  else {
    toggle_LED = false;
    digitalWrite(SCK_LED, LOW);    // Pilot LED OFF
  }
  //}

  delay(500);
  if (any_fail_flag == true) {
    power_on = true;
    any_fail_flag = false;
  }
  else power_on = false;
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

void reconnect_to_WiFi() {
  Serial.print("Re-connecting WiFi to ");
  Serial.println(wifissid);

  WiFi.begin(wifissid, wifipass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  debug = 0;
  any_fail_flag = true;
  delay(500);
}

