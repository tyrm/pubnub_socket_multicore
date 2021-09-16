#include <M5Atom.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>

#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

#include "secrets.h"

typedef struct SerialMessage {
  char body[20];
} SerialMessage;

// led
uint8_t disBuff[2 + 5 * 5 * 3];
bool blinkState = true;

// wifi config
const char* wifiSSID = WIFI_SSID;
const char* wifiPass = WIFI_PASSWORD;

// queues
int queueSize = 10;

QueueHandle_t cmdQueue;
QueueHandle_t serQueue;

// punub
const char* pnPubKey = PN_PUB_KEY;
const char* pnSubKey = PN_SUB_KEY;

char uuid[]        = "00000000-0000-0000-0000-000000000000";
char commandChan[] = "command.00000000-0000-0000-0000-000000000000";
char stateChan[]   = "state.00000000-0000-0000-0000-000000000000";

void setup() {
  setup_hardware();
  setup_wifi();
  setup_pubsub();

  // create command queue
  cmdQueue = xQueueCreate( queueSize, sizeof( bool ) );
  if(cmdQueue == NULL){
    Serial.println("Error creating the command queue");
  }
 
  // create serial message queue
  serQueue = xQueueCreate( queueSize, sizeof( SerialMessage ) );
  if(serQueue == NULL){
    Serial.println("Error creating the serial messagequeue");
  }
 
  xTaskCreatePinnedToCore(
    subscriberTask,     /* Task function. */
    "subscriber",       /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL, 1);            /* Task handle. */
 
  xTaskCreatePinnedToCore(
    mainTask,     /* Task function. */
    "main",       /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL, 0);            /* Task handle. */
 
  // Delete "setup and loop" task
  vTaskDelete(NULL);
}
 
void loop() {
}
 
void subscriberTask( void * parameter )
{
  SerialMessage msg;
  StaticJsonDocument<254> pubjd;
  while (1) {
    strcpy(msg.body, "waiting for message");
    xQueueSend(serQueue, (void *)&msg, 10);

    PubSubClient* subclient = PubNub.subscribe(commandChan);
    if (!subclient) {
      strcpy(msg.body, "subscription error");
      xQueueSend(serQueue, (void *)&msg, 10);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    
    String           msg;
    SubscribeCracker ritz(subclient);
    while (!ritz.finished()) {
      ritz.get(msg);
      if (msg.length() > 0) {
        //Serial.print("Received: ");
        //Serial.println(msg);
        
        pubjd.clear();
        deserializeJson(pubjd, msg);
  
        if (pubjd.containsKey("state")) {
          bool state = pubjd["state"].as<bool>();
          xQueueSend(cmdQueue, &state, 0);
        }
      }
    }
  
    subclient->stop();
  }
  
  //vTaskDelay(10 / portTICK_PERIOD_MS);
}
 
void mainTask( void * parameter)
{
  bool command;
  SerialMessage rcv_msg;
  while (1) {
    loop_hardware();
    
    // See if there's a command in the queue (do not block)
    if (xQueueReceive(cmdQueue, (void *)&command, 10) == pdTRUE) {
      if (command) {
        Serial.print("true|");
      } else {
        Serial.print("false|");
      }
    }
    
    // See if there's a serial message in the queue (do not block)
    if (xQueueReceive(serQueue, (void *)&rcv_msg, 10) == pdTRUE) {
      Serial.println(rcv_msg.body);
    }
  }
}
