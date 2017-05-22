Connecting Node-RED and ESP8266 Wemos D1 using MQTT
====================================================

The sketch publishes several sensor data including luminocity and temperature (simulated) through MQTT protocol.

It also listens to MQTT topics with MQTT callback routine. When LED_ALERT is received, it turns on or off of an LED based on the Boolean value of the alert.

Thus, it requires an MQTT server.

created May 20, 2017
by Indy Star
