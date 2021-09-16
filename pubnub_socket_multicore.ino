#include <M5Atom.h>
#include "AtomSocket.h"
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

// punub
const char* pnPubKey = PN_PUB_KEY;
const char* pnSubKey = PN_SUB_KEY;

char uuid[]        = "00000000-0000-0000-0000-000000000000";
char commandChan[] = "command.00000000-0000-0000-0000-000000000000";
char stateChan[]   = "state.00000000-0000-0000-0000-000000000000";

// queues
int queueSize = 10;

QueueHandle_t cmdQueue;
QueueHandle_t serQueue;

// socket
#define RXD 22
#define RELAY 23

bool           socketDesiredState = false;
bool           socketState = false;
ATOMSOCKET     socket;
HardwareSerial AtomSerial(2);

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
    subscriberTask,   /* Task function. */
    "subscriber",     /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL, 1);         /* Task handle. */
 
  xTaskCreatePinnedToCore(
    mainTask,         /* Task function. */
    "main",           /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL, 0);         /* Task handle. */
 
  // Delete "setup and loop" task
  vTaskDelete(NULL);
}
 
void loop() {
}
 
void subscriberTask( void * parameter )
{
  SerialMessage           serMsg;
  StaticJsonDocument<254> pubjd;
  while (1) {
    strcpy(serMsg.body, "waiting for message");
    xQueueSend(serQueue, (void *)&serMsg, 10);
    
    PubSubClient* subclient = PubNub.subscribe(commandChan);
    if (!subclient) {
      strcpy(serMsg.body, "subscription error");
      xQueueSend(serQueue, (void *)&serMsg, 10);
      
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    
    String           msg;
    SubscribeCracker ritz(subclient);
    while (!ritz.finished()) {
      ritz.get(msg);
      if (msg.length() > 0) {
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
  
  set_dis_buff(0xff, 0x00, 0x00);
  M5.dis.displaybuff(disBuff);
  
  while (1) {
    loop_hardware();
    
    // See if there's a command in the queue (do not block)
    if (xQueueReceive(cmdQueue, (void *)&command, 10) == pdTRUE) {
      if (command) {
        socketDesiredState = true;
        Serial.println("cmd: state=true");
      } else {
        socketDesiredState = false;
        Serial.println("cmd: state=false");
      }
    }
    
    // See if there's a serial message in the queue (do not block)
    if (xQueueReceive(serQueue, (void *)&rcv_msg, 10) == pdTRUE) {
      Serial.println(rcv_msg.body);
    }
  }
}
