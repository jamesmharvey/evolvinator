// 4 Time -----------------------------------------------------------

// Setting time to NTP server
unsigned long getTime() {
  if (debugMode) Serial.println("Time being synced via NTP");
  sendNTPpacket(timeServer);                                           // send an NTP packet to a time server
  delay(1000);                                                         // wait to see if a reply is available
  if (Udp.parsePacket()) {  
    Udp.read(packetBuffer, NTP_PACKET_SIZE);                           // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]); // get timestamp words
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    unsigned long secsSince1900 = highWord << 16 | lowWord;            // combine into integer (seconds since Jan 1 1900)
    unsigned long seventyYears = 2208988800UL;                         // change to unix time 
    tUnix = secsSince1900 - seventyYears;                              // record unix time   
    if (debugMode) {
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(secsSince1900);
      Serial.print("Unix time = ");
      Serial.println(tUnix);
      Serial.print("Human readable time: ");
      Serial.print(year(tUnix));
      Serial.print("-");
      Serial.print(month(tUnix));
      Serial.print("-");
      Serial.print(day(tUnix));
      Serial.print(" ");
      Serial.print(hour(tUnix));
      Serial.print(":");
      if (minute(tUnix) < 10) {
        Serial.print("0");
      }
      Serial.print(minute(tUnix));
      Serial.print(":");
      if (second(tUnix) < 10) {
        Serial.print("0");
      }
      Serial.print(second(tUnix));
      Serial.println(" UTC");
    }
    return tUnix;
  }
}

// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(byte *address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  Udp.beginPacket(address, 123);		   
  Udp.write(packetBuffer,NTP_PACKET_SIZE); //NTP requests are to port 123
  Udp.endPacket();
}

// timeCheck
void timeCheck() {
  t = now();
  if (year(t) >= 2024 && currentMs - msBackup > 5 * 60000) {
    tBackup = now();
    msBackup = millis();
    if (debugMode) {
      Serial.print("Back up time: ");
      Serial.println(tBackup);
    }
  }
  if (year(t) < 2024) {
    if (debugMode) {
      Serial.println("Year is wrong: ");
      Serial.println(year(t));
    }
    setTime(hour(tBackup), minute(tBackup), second(tBackup), day(tBackup), month(tBackup), year(tBackup));
    int adjustTime = (currentMs - msBackup) / 1000;
    setTime(hour(tBackup), minute(tBackup) + adjustTime / 60, second(tBackup) + adjustTime % 60, day(tBackup), month(tBackup), year(tBackup));
    t = now();
    if (debugMode) {
      Serial.println("Time failed to reset correctly, backup used");
      Serial.print("Is year right? ");
      Serial.println(year(t));
    }
  }
  if (tStart) {
    tElapsed = t - tStart;
    tUnix = tUnixStart + tElapsed;
  }
}
