# Friot_cpp_finalupdate
Friot stands for Front-End Request IoT, it was a 2019-2020 senior project done by a group of six people. 
For more details, visit this [Youtube Video](https://youtu.be/3iLt__HzNMs?list=PLNV8XAzo8y_3RyNA_sNPMYtLZNhUcbWxa) or the [Website](https://friot.live/)

This repository includes only the code for the ESP8266 chip. 
It works fine without the actual App since it is using primarily HTTP GET/POST requests.

### Setup
You will need 8 libraries to run the code properly, in which most of them can be found in the ESP8266 Package except for the first two libraries (I've included the links)
- [FirebaseESP8266.h](https://github.com/mobizt/Firebase-ESP8266)
- [TimeLib.h](https://github.com/PaulStoffregen/Time)
- ESP8266WiFi.h
- WiFiClient.h
- ESP8266WebServer.h
- DNSServer.h
- ESP8266mDNS.h
- EEPROM.h

If you have not already created a Firebase account please sign up for one and come back for the next step after you have a realtime database configured.

You should modify these lines with your database URL(found on General Setting) and the Database Secrets(found on Service Account Setting)
```
#define FIREBASE_HOST "HOST"
#define FIREBASE_AUTH "AUTH KEY"
```

This line determines the type of the device, current there are only two, ON/OFF type, and 9 colors LED type
```
#define DeviceType "ON/OFF" //Type[] "ON/OFF","LED"
```
### ESP8266 Pins Setup
This code is using 3 output pins, D0,D1, and D2. each represents R G B color code respectively. For a lamp device, it is only using pin D0 to control a High or Low signal.
This configuration can be easily modified under the LedOnChip() funciton
```
void LedOnChip(){
  //output setup
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
}
```
### Connecting the Device to Internet
When you turn on the device, it will first search for other existing device. If no device is found, you will have access to the device from your phone or from you personal computer.
you will have to select your Wi-Fi and input the Wi-Fi password. If you have done that to a different device earlier, then this device will automatically receive all the Wi-Fi information and connect to the internet.

To change the status of the device (from On to Off or vice versa), access it through browser. 
Such as creating a post request to http://[ip of the device]/receive. For example, in a html,
```
<form action="http://192.168.0.197/receive" method="post" target="_blank">
```
