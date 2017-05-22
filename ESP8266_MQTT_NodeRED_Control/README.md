Connecting Node-RED and ESP8266 Wemos D1 using MQTT
====================================================

The sketch publishes several sensor data including luminosity and temperature (currently simulated) through MQTT protocol.

It also listens to MQTT topics with MQTT callback routine. When LED_ALERT topic is received, it turns on or off of an LED based on the Boolean value of the alert.

It requires an MQTT server.

created May 20, 2017
by Indy Star
