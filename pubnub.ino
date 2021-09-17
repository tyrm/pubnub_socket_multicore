void pub_state(bool state) {
  char stateMsg[15];

  if (state) {
    strcpy(stateMsg, "{\"state\":true}");
  } else {
    strcpy(stateMsg, "{\"state\":false}");
  }
  
  PubNonSubClient *pclient = PubNub.publish(stateChan, stateMsg);
}

void setup_pubsub() {
  uint32_t chipId    = 0;
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(uuid, sizeof(uuid), "dad6d7a1-bf1f-433d-bcb5-%012lx", chipId);
  snprintf(commandChan, sizeof(commandChan), "command.%s", uuid);
  snprintf(stateChan, sizeof(stateChan), "state.%s", uuid);

  Serial.print(millis(), DEC);
  Serial.print(" - PubNub: my uuid is: "); Serial.println(uuid);
  
  PubNub.begin(pnPubKey, pnSubKey);
  PubNub.set_uuid(uuid);
  Serial.print(millis(), DEC);
  Serial.println(" - PubNub: set up complete");
}
