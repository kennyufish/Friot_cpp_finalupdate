/* pinout configuration notes
D0,D1,D2
RGB
000:white
001:yellow
010:Magenta
011:red
100:cyan
101:green
110:blue
111:off
*/

void Output(String Type, String Status){
	//if its a device that turns on and off;
	if (Type == "ON/OFF"){
		if (Status == "on"){
			digitalWrite(outputPin, HIGH);
			UpdateStatus(Status);
		}
		if (Status == "off"){
			digitalWrite(outputPin, LOW);
			UpdateStatus(Status);
		}
		
	}
	
	//if the device has multiple outputs, like LED in this case
	if (Type == "LED"){
		//convert the status string to color code string
		String ColorCode = ColorStringToColorBinCode(Status);
		//if it says on which's technically white color
		if (Status == "on")
			ColorCode = "000";
		if(ColorCode != "1000"){
      Serial.println(ColorCode);
      Serial.println(ColorCode[0]);
      Serial.println(ColorCode[1]);
      Serial.println(ColorCode[2]);
      Serial.println(ColorCode[3]);
			digitalWrite(D0,int(ColorCode[0]-'0'));	//R signal
			digitalWrite(D1,int(ColorCode[1]-'0'));	//G signal
			digitalWrite(D2,int(ColorCode[2]-'0'));	//B signal
			UpdateStatus(Status);
		}
	}
}

//======Wifi conneting function=====//
void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}
int intdata(FirebaseData &data){
  return data.intData();
  }

//====Firebase Print debug tool
void printResult(FirebaseData &data)
{

    if (data.dataType() == "int")
        Serial.println(data.intData());
    else if (data.dataType() == "float")
        Serial.println(data.floatData(), 5);
    else if (data.dataType() == "double")
        printf("%.9lf\n", data.doubleData());
    else if (data.dataType() == "boolean")
        Serial.println(data.boolData() == 1 ? "true" : "false");
    else if (data.dataType() == "string")
        Serial.println(data.stringData());
    else if (data.dataType() == "json")
    {
        Serial.println();
        FirebaseJson &json = data.jsonObject();
        //Print all object data
        Serial.println("Pretty printed JSON data:");
        String jsonStr;
        json.toString(jsonStr,true);
        Serial.println(jsonStr);
        Serial.println();
        Serial.println("Iterate JSON data:");
        Serial.println();
        size_t len = json.iteratorBegin();
        String key, value = "";
        int type = 0;
        for (size_t i = 0; i < len; i++)
        {
            json.iteratorGet(i, type, key, value);
            Serial.print(i);
            Serial.print(", ");
            Serial.print("Type: ");
            Serial.print(type == JSON_OBJECT ? "object" : "array");
            if (type == JSON_OBJECT)
            {
                Serial.print(", Key: ");
                Serial.print(key);
            }
            Serial.print(", Value: ");
            Serial.println(value);
        }
        json.iteratorEnd();
    }
    else if (data.dataType() == "array")
    {
        Serial.println();
        //get array data from FirebaseData using FirebaseJsonArray object
        FirebaseJsonArray &arr = data.jsonArray();
        //Print all array values
        Serial.println("Pretty printed Array:");
        String arrStr;
        arr.toString(arrStr,true);
        Serial.println(arrStr);
        Serial.println();
        Serial.println("Iterate array values:");
        Serial.println();
        for (size_t i = 0; i < arr.size(); i++)
        {
            Serial.print(i);
            Serial.print(", Value: ");

            FirebaseJsonData &jsonData = data.jsonData();
            //Get the result data from FirebaseJsonArray object
            arr.get(jsonData, i);
            if (jsonData.typeNum == JSON_BOOL)
                Serial.println(jsonData.boolValue ? "true" : "false");
            else if (jsonData.typeNum == JSON_INT)
                Serial.println(jsonData.intValue);
            else if (jsonData.typeNum == JSON_DOUBLE)
                printf("%.9lf\n", jsonData.doubleValue);
            else if (jsonData.typeNum == JSON_STRING ||
                     jsonData.typeNum == JSON_NULL ||
                     jsonData.typeNum == JSON_OBJECT ||
                     jsonData.typeNum == JSON_ARRAY)
                Serial.println(jsonData.stringValue);
        }
    }
}

//=================
void UpdateStatus(String stat){
	//update currentstatus on chip
	CurrentStatus = stat;
	//updates log on the standalone node
	Serial.println("Updating Status...");
	Firebase.setString(firebaseData, DeviceFactoryName + "/Device_Status/STATUS" , CurrentStatus);
    Firebase.setString(firebaseData, DeviceFactoryName + "/LOG/" + logtime() , CurrentStatus);
	//update pie status
	Firebase.getInt(firebaseData,  DeviceFactoryName + "/PIE/" + stat);
	Firebase.setInt(firebaseData,  DeviceFactoryName + "/PIE/" + stat, firebaseData.intData() + 1);
}


//setup the pins////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////

// Chip searches for a main chip/////////////////////
void searchForMain(){
  //search for five times in case missing SSID sometimes
  for (int j = 0; j < 5; j++){
    int n = WiFi.scanNetworks();
    if (n > 0) {
      for (int i = 0; i < n; i++) {
        Serial.println( WiFi.SSID(i) + " :: " + WiFi.encryptionType(i) + " :: " + WiFi.RSSI(i));
    //if we find a main, try to connect
        if(WiFi.SSID(i).substring(0,7) == "ESP8266") {
          Serial.println( WiFi.SSID(i) + " FOUND!");
      //write down the main AP name
      MainSSID = WiFi.SSID(i);
      //turn on EP mode
          EP_MODE(WiFi.SSID(i));
          Epmode = true;
          break; 
        }
      }
    }
    if (Epmode == true) break; //break 5x search loop;
  }
  //if no main is found, turn on Main mode
  if (Epmode != true){
      Serial.println("\nMain Mode activating...\n");
      Mainmode = true;
      MainMode();
    }
}

void EP_MODE(String ssidname){
  Serial.println("\nActivating EP MODE\n");
  //set to station mode
  WiFi.mode(WIFI_STA);
  //Input SSID and psk for Main access
  ssidname.toCharArray(ssid, sizeof(ssid) - 1);
  //current plan is to have AP open wifi access, so we dont need password
  //String ssidpw = APPSK;
  //ssidpw.toCharArray(password, sizeof(password) - 1);
}

void MainMode(){
  //set to ap and station mode
  WiFi.mode(WIFI_AP_STA);           
  //create soft AP for main
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  //WiFi.softAP(softAP_ssid, softAP_password);
  WiFi.softAP(softAP_ssid);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  CreateServer();
  
  //auto signin if previously signed in
  loadCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
}
void CreateServer(){
  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  // Web server
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/wifiinfo", HTTP_GET, handleWifiInfo);
  server.on("/getip", HTTP_GET, handleGetIP);
  server.on("/changename", HTTP_POST, handleName);
  server.on("/receive", HTTP_POST, handleCommand);
  server.onNotFound(handleNotFound);
  
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  dnsServer.start(DNS_PORT, "*", apIP);
}


//function_list tab
int ObtainWIFIinfo(){
  //if it is on EP mode, we start a client connection
  if (Epmode == true){
    WiFiClient Ep_client;
  //IPAddress newip = resolver.search(MainSSID);
    String  host = MainSSID;
    Serial.println("\n[Connecting to " + host + " ... ");

    ///if connection made
    if (Ep_client.connect(host, 80)){
      Serial.println("connected]");
      Serial.println("[Sending a request]");
      Ep_client.print(String("GET /wifiinfo") + " HTTP/1.1\r\n" +"Host: " 
    + host + "\r\n" + "Connection: close\r\n" + "\r\n" );
  
      Serial.println("[Response:]");
    //while we have something to receive from the server
      while (Ep_client.connected() || Ep_client.available()){
        if (Ep_client.available()){
          String line = Ep_client.readStringUntil('\n');
          WifiInfoCheck(line);
          Serial.println(line);
        }
      }
    //stop the client once its done
      Ep_client.stop();
      Serial.println("\n[Disconnected]");
      delay(5000);
      return 1;
    }
    else{
  // if we fail to connect, stop regardless and reutnr 0
      Serial.println("connection failed!]");
      Ep_client.stop();
      delay(5000);
      return 0;
    }
  }
}

//function_list tab
void WifiInfoCheck(String key){
  if(key.substring(0,5) == "SSID:"){
     key = key.substring(5,key.length());
     Serial.println("\nSeting SSID as: " + key + "\n");
     //Input SSID and psk for Main access
     key.toCharArray(ssid, sizeof(ssid) - 1);
  }
  if(key.substring(0,9) == "PASSWORD:"){
     key = key.substring(9,key.length());
     Serial.println("\nSeting PASSWORD as: " + key + "\n");
     //Input SSID and psk for Main access
     key.toCharArray(password, sizeof(password) - 1);
  }
}


//handleHttp tab
void handleWifiInfo() {
  String tempssid(ssid);
  String temppas(password);
  server.send(200, "text/plain", "SSID:" + tempssid + "\n\n");
  server.send(200, "text/plain", "PASSWORD:" + temppas);
  Serial.println("Wifiinfo SENT!");
}
