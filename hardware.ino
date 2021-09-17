void loop_hardware() {
  M5.update();

  if (M5.Btn.wasPressed()) {
    socketDesiredState = !socketDesiredState;
  }
  
  if (socketState != socketDesiredState) {
    if (socketDesiredState) {
      socketState = true;
      set_dis_buff(0x00, 0xff, 0x00);
      M5.dis.displaybuff(disBuff);
      socket.SetPowerOn();
      Serial.print(millis(), DEC);
      Serial.println(" - turned on");
      pub_state(true);
    } else {
      socketState = false;
      set_dis_buff(0xff, 0x00, 0x00);
      M5.dis.displaybuff(disBuff);
      socket.SetPowerOff();
      Serial.print(millis(), DEC);
      Serial.println(" - turned off");
      pub_state(true);
    }
  }

  // check for reading
  socket.SerialReadLoop();
  if(socket.SerialRead == 1 && millis() > lastStatsSend + 5000) {
    lastStatsSend = millis();
    socketVoltage = socket.GetVol();
    socketCurrent = socket.GetCurrent();
    socketActivePower = socket.GetActivePower();
    Serial.print("Voltage: ");
    Serial.print(socketVoltage);
    Serial.println(" V");
    Serial.print("Current: ");
    Serial.print(socketCurrent);
    Serial.println(" A");
    Serial.print("ActivePower: ");
    Serial.print(socketActivePower);
    Serial.println(" W");
  }
}

void set_dis_buff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata) {
  disBuff[0] = 0x05;
  disBuff[1] = 0x05;
  for (int i = 0; i < 25; i++)
  {
    disBuff[2 + i * 3 + 0] = Rdata;
    disBuff[2 + i * 3 + 1] = Gdata;
    disBuff[2 + i * 3 + 2] = Bdata;
  }
}

void setup_hardware() {
  // Start Serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println();

  // Start M5 Hardware
  M5.begin(true, false, true);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  set_dis_buff(0x00, 0x00, 0xff);
  M5.dis.displaybuff(disBuff);

  // start Socket
  socket.Init(AtomSerial, RELAY, RXD);
}
