/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

String stringYearMonthDay(){
  return (String(year()) + "." + String(month()) + "." + String(day()));
  }

String logtime(){
  String realtime;
  realtime = realtime + String(hour());
  realtime = realtime + ":";
  if (minute() < 10)
    realtime = realtime + "0";
  realtime = realtime + String(minute());
  realtime = realtime + ":";
  if (second() < 10)
    realtime = realtime + "0";
  realtime = realtime + String(second());
  return ("/" + String(year()) + "-" + String(month()) + "-" + String(day()) + "/" + realtime);
  }

//==========search, locate the color
int SearchColor(String keyword ){
  for (int i = 0; i <ColorNum ;i++){
    if (keyword == LedColors[i]){
      return i;
      break;
    }
  }
  // 8 means the color doesnt exist
  return 8;
}
//====change location from dec to bin command
int DecToBin(int n){
  int binary = 0;
  int rem,temp = 1;
  while (n != 0)
  {
    rem = n % 2;
    n = n / 2;
    binary = binary + rem * temp;
    temp = temp *10;
   }
    return binary;
}
//================
String ColorStringToColorBinCode(String color){
  switch (SearchColor(color)){
    case 0:
      return "000";
      break;
    case 1:
      return "001";
      break;
    case 2:
      return "010";
      break;
    case 3:
      return "011";
      break;
    case 4:
      return "100";
      break;
    case 5:
      return "101";
      break;
    case 6:
      return "110";
      break;
    case 7:
      return "111";
      break;
    default: 
      return "1000";  
    }
  }
