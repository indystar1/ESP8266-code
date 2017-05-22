# Connecting ESP8266 Wemos D1 to AT&T M2X Platform with Flow
==========================================================

The sketch utilizes [M2XArduinoStreamClient](https://github.com/indystar1/M2XArduinoStreamClient).

It connects to AT&T M2X platform using Device ID, Stream ID, and your PRIMARY API KEY.

It sends simulated Celcius temperature values to M2X.

By using a GET RESTful call to ATT M2X server, it also polls "control" value of another stream, which is used to turn on or off of an LED on board.

Because M2X returns a JSON packet, the sketch uses [aJSON library](https://github.com/interactive-matter/aJson) to extract the value of _control_.

But the response also includes (start with) HTTP header, the header needs to be removed. For this purpose, I used a very simple detection method (character comparision) to find out the beginning of JSON string and then used aJSON library to extract _control_ value.

created May 20, 2017
by Indy Star
