void setup_wifi() {
  Serial.print(millis(), DEC);
  Serial.print(" - Wifi: connecting to SSID \"");
  Serial.print(wifiSSID);
  Serial.print("\"");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPass); 

  
  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (blinkState) {
      set_dis_buff(0x00, 0x00, 0x00);
      M5.dis.displaybuff(disBuff);
      blinkState = false;
    } else {
      set_dis_buff(0x00, 0x00, 0xff);
      M5.dis.displaybuff(disBuff);
      blinkState = true;
    }
    // wait 1 second for re-trying
    delay(500);
  }

  WiFi.setAutoReconnect(true);

  Serial.println();
  Serial.print(millis(), DEC);
  Serial.print(" - Wifi: Connected to ");
  Serial.println(wifiSSID);
  Serial.print(millis(), DEC);
  Serial.print(" - Wifi: IP address: ");
  Serial.println(WiFi.localIP());
  
  set_dis_buff(0x00, 0xff, 0x00);
  M5.dis.displaybuff(disBuff);
  delay(250);
  set_dis_buff(0x00, 0x00, 0x00);
  M5.dis.displaybuff(disBuff);
  delay(250);
  set_dis_buff(0x00, 0xff, 0x00);
  M5.dis.displaybuff(disBuff);
  delay(250);
  set_dis_buff(0x00, 0x00, 0x00);
  M5.dis.displaybuff(disBuff);
  delay(250);
}
