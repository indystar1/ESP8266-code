Arduino to Thingworx REST POST and GET using ESP8266
====================================================

Advanced REST-API test url for posting four properties:
```
http://academic.cloud.thingworx.com/Thingworx/Things/ESP8266_REST_yours/Services/setTempAndHumid?appKey=your_app_key&method=post&Temp=53.70&Humid=34.50&Bool=true&Analog=123.45 (Content-Type: application/json)
```

Advanced REST-API test url for getting a control command:
```
http://academic.cloud.thingworx.com/Thingworx/Things/ESP8266_REST_yours/Properties/YourCtrl?method=get&appKey=your_app_key (Content-Type: application/json)
```

This is based on [Weather App with Arduino Uno](https://www.thingworx.com/ecosystem/academic-program/iot-projects/weather-app-arduino-uno/)

The sketch reads four (two simulated) sensor values and sends to a Thing on ThingWorx.

It also gets a value from Mashup to control a device such as LED.

On the Thingworx side, make sure your thingName[], your serviceName[] and your server[] match exactly.

Use your provided appKey that you receive when you set up your Thing.

Create a service in your Thing or ThingShape and create inputs that will match the variable names you use in your Arduino code.  In the JavaScript window of Services, type something similar to the following:

```
me.Temperature = parseFloat(Temp);
me.Humidity = parseFloat(Humid);
me.BoolStatus = parseBoolean(Bool);
me.AnalogValue = parseFloat(Analog);
```

Everything is case sensitive.

It uses Arduino ESP8266 library and [aJson code](https://github.com/interactive-matter/aJson/)

created 4/3/2016
modified 3/23/2017
by Indy Star

This code was based on
Arduino to Thingworx Ethernet Web Client using Ethernet Shield
by Nick Milleson
Design Engineer
EAC Product Development Solutions