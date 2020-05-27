
//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <TimeLib.h>

//Use for Lamp configuration///////////////////


//Settings that are changeable////////////////////////////////////////////////////////
  //===Friot-Images Firebase===//
  #define FIREBASE_HOST "HOST"
  #define FIREBASE_AUTH "AUTH KEY"
  //===Firebase===//
  //Device Setting
  String DeviceNumber = "1" ; //hardcoded number for easier implementation
  String DeviceFactoryName = "FRIoT-LAMP-TEST"; 
  String DeviceName = "Lamp"; //changeable at runtime
  String CurrentStatus = "off"; //initial status is off
  const char *myHostname = "FRIoT-LAMP-TEST"; // hostname used for Mdns
  #define DeviceType "ON/OFF" //Type[] "ON/OFF","LED"
  #define outputPin D0 //for ON/OFF devices
  
///////////////////////////////////////////////////

//Settings for AP modes//////////////////////////////////////////////////////
  #define APSSID "FRIoT-LAMP-TEST"/* Set these to your desired softAP credentials. They are not configurable at runtime */
  #define APPSK  "12345678" //not using, just direct connection in our case
  const char *softAP_ssid = APSSID;
  const char *softAP_password = APPSK;
  /* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
  char ssid[32] = "";
  char password[32] = "";
//////////////////////////////////////////////////////

//Settings for LED devices//////////////////////////////////////////////////////
#define ColorNum 8             //total amount of colors
String LedColors[ColorNum] { "white", "yellow", "magenta", "red", "cyan" ,"green", "blue", "off"};
int ColorPie[ColorNum];
//////////////////////////////////////////////////////

//web server settings and firebase data object//////////////////////////////////////////////////////
  // DNS server
  const byte DNS_PORT = 53;
  DNSServer dnsServer;
  // Web server
  ESP8266WebServer server(80);
  /* Soft AP network parameters */
  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMsk(255, 255, 255, 0);
  /** Should I connect to WLAN asap? */
  boolean connect;
  /** Last time I tried to connect to WLAN */
  unsigned long lastConnectTry = 0;
  /** Current WLAN status */
  unsigned int status = WL_IDLE_STATUS;
  //Define FirebaseESP8266 data object
  FirebaseData firebaseData;
  FirebaseJson json;
////////////////////////////////////////////////////////

// NTP Servers and time//////////////////////////////////////////////////////
  IPAddress timeServer;
  const char* ntpServerName = "0.north-america.pool.ntp.org";
  const int timeZone = -8;  // Pacific Standard Time (USA)
  unsigned int localPort = 8888;  // local port to listen for UDP packets
  WiFiUDP Udp;
////////////////////////////////////////////////////////

//Main and EP settings//////////////////////////////////////////////////////
  bool Epmode = false;
  bool Mainmode = false;
  bool RealInternet = false;
  String MainSSID = "OFF";
////////////////////////////////////////////////////////

//====setup===////////////////////////////////////////////////////////
void setup() {
  LedOnChip();
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  //ClearCredentials();
  searchForMain();
  Serial.println("Finished Searching Main...");
}

//loop for the chip////////////////////////////////////////////////////////
void loop() {
  //check for any connection made to the chip
  if (connect) {
    Serial.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
    Serial.println("loaded millis()");
  }
  //if there isnt a connection
  {
    //check for wifi status
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
		if (s == WL_CONNECTED) {
			/* Just connected to WLAN */
			Serial.print("Connected to ");
			Serial.println(ssid);
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());
			//while its on EP mode and its not on the real internet connection
        while (Epmode == true && RealInternet == false){
			//attempts to get wifi info from Main AP
			if(ObtainWIFIinfo()){
				//check for wifi status and reloads
				Serial.println(WiFi.status());
				Serial.println("Disconnecting from AP");
				Serial.println(WiFi.status());
				//turn on the flag for real internet 
				RealInternet = true;
				//create servers for access and take commands
				CreateServer();
				Serial.println("Connecting to Real Internet");
            }
			//delay here is for stablized connection
			delay(2500);
        }

        //For getting a real time from a ntp server
        WiFi.hostByName(ntpServerName,timeServer);
        Udp.begin(localPort);
        while(!getNtpTime);
		setSyncProvider(getNtpTime);
        
        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
			Serial.println("Error setting up MDNS responder!");
        } else {
			Serial.println("mDNS responder started  `");
			// Add service to MDNS-SD
			MDNS.addService("http", "tcp", 80);
        }

        //Firebase setup
        Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
        Firebase.reconnectWiFi(true);
        //Set database read timeout to 1 minute (max 15 minutes)
        Firebase.setReadTimeout(firebaseData, 1000 * 60);
        //tiny, small, medium, large and unlimited.
        //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
        Firebase.setwriteSizeLimit(firebaseData, "tiny");

        //For phone access data nodes
        //setting Devices/0/INDEX
        Firebase.setString(firebaseData,  "Devices/" + DeviceNumber + "/INDEX" , DeviceNumber);
        //setting Devices/0/IP
        Firebase.setString(firebaseData,  "Devices/" + DeviceNumber + "/IP" , toStringIp(WiFi.localIP()));   
        //setting Devices/0/NAME
        Firebase.setString(firebaseData,  "Devices/" + DeviceNumber + "/NAME" , DeviceName);
        //setting Devices/0/STATUS
        if (Firebase.getString(firebaseData,  "Devices/" + DeviceNumber + "/STATUS"))
          CurrentStatus = firebaseData.stringData(); //update status if theres any;

        //standlone data nodes
        Firebase.setString(firebaseData,  DeviceFactoryName + "/Device_Status/INDEX" , DeviceNumber);
        Firebase.setString(firebaseData,  DeviceFactoryName + "/Device_Status/IP" , toStringIp(WiFi.localIP()));
        Firebase.setString(firebaseData,  DeviceFactoryName + "/Device_Status/NAME" , DeviceName);
        Firebase.setString(firebaseData,  DeviceFactoryName + "/Device_Status/STATUS" , CurrentStatus);
        Serial.println("Current status on the Device: " + CurrentStatus);
        //update output
        Output(DeviceType,CurrentStatus);
}
    else if (s == WL_NO_SSID_AVAIL) {
		WiFi.disconnect();
    }
    }
    if (s == WL_CONNECTED) {
      //Serial.println("MDS update()");
      MDNS.update();
      //Serial.println("MDS success()");
    }
  }
  
	//DNS
	dnsServer.processNextRequest();
	//HTTP
	server.handleClient();

	//Loop to constantly check for updates from firebase
	//get firebase device status
	if(Firebase.getString(firebaseData, "Devices/" + DeviceNumber + "/STATUS")){
		//check for firebase device status, if doesnt match with local, make adjustment
		if (CurrentStatus != firebaseData.stringData()){
			Serial.println("Status change on firebase detected: " + firebaseData.stringData());
			CurrentStatus = firebaseData.stringData();
			Output(DeviceType,CurrentStatus);
		}
	}
   
	//get firebase device name, change if necessary
	if (Firebase.getString(firebaseData,  "Devices/" + DeviceNumber + "/NAME" )){
		if (DeviceName != firebaseData.stringData()){
  			DeviceName = firebaseData.stringData();
			  Firebase.setString(firebaseData,  DeviceFactoryName + "/Device_Status/NAME" , DeviceName);
			  Serial.println("Name has changed to " + DeviceName);
		}
	}
}
