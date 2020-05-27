
/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F("<html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><head></head><body>");
  Page += F("<h1 style=\"background-color:Tomato;\">Welcome to FRIoT</h1>");
  Page += F("<div style=\"background-color:MediumSeaGreen;\"> ");
  Page += F("<p><br><a href='/'>Home</a> | <a href=\"/wifi\">WiFi Setting</a> |  <a href=\"/Contact/\">Contact</a></p>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p><br>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p><br>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page += F("<p style=\"background-color:MediumSeaGreen;\">You may want to <a href='/wifi'>config the wifi connection</a>&#128512;<br></p></div></body></html>");
  server.send(200, "text/html", Page);
}
/** Wifi config page handler */
void handleWifi() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += String(F("<html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><head></head><body>")) +
          F("<h1 style=\"background-color:Tomato;\">Wi-Fi Configuration</h1>");
  Page += F("<div style=\"background-color:LightBlue;\"> ");
  Page += F("<p><br><a href='/'>Home</a> | <a href=\"/wifi\">WiFi Setting</a> |  <a href=\"/Contact/\">Contact</a></p>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F("<p>WLAN list (<a href='/wifi'>Refresh</a> if any missing)</p>"));
    
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("<button type='button' onclick='ChangeText(\""))+ WiFi.SSID(i) + F("\")'>") + 
      WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + 
      F(" (") + WiFi.RSSI(i) + F(")</button><br>");
    }
  } else {
    Page += F("<p>No WLAN found</p>");
  }
  Page += F("<form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' id='ssid' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            );
  Page += String(F("<p>=====================</p>" "<table><tr><th align='left'>SoftAP config</th></tr>"
            "<tr><td>SSID ")) + String(softAP_ssid) +
          F("</td></tr>"
             "<tr><td>IP ") + toStringIp(WiFi.softAPIP()) +
          F("</td></tr>" "</table>"
            "\r\n<br />"
            "<table><tr><th align='left'>WLAN config</th></tr>"
            "<tr><td>SSID ") +  String(ssid) +
          F("</td></tr>"
            "<tr><td>IP ") + toStringIp(WiFi.localIP()) +
          F("</td></tr>" "</table>"
             "<script>"
            "function ChangeText(name_ssid) {"
               "document.getElementById('ssid').value = name_ssid;}"
            "</script>"
            "</body></html>"
            );
  
  Page += F("</div>");
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}



//==========color command response
void handleCommand() {
  String device = server.arg("OutputDevice");
  String Status = server.arg("Command");
  Output(DeviceType,Status);
}
//=============//

void handleGetIP() {
  if (WiFi.localIP().toString() != "(IP unset)")
    server.send(200, "text/plain", WiFi.localIP().toString());
  else
    server.send(200, "text/plain", "Server is not connected to a WiFi! IP: " + WiFi.localIP().toString());
  Serial.println("Requested IP : " + WiFi.localIP().toString());
}

//====NameChanging=====
void handleName(){
  String NameChange = server.arg("Name");
  Firebase.setString(firebaseData, DeviceFactoryName + "/Device_Status/Name", NameChange);
  server.send(200, "text/plain", "Successfully Changed Name to : " + NameChange );
  Serial.println("New Name : " + NameChange);
}
