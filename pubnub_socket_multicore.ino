#include <M5Atom.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

#include "secrets.h"

// Message struct: used to wrap strings (not necessary, but it's useful to see
// how to use structs here)
typedef struct Message {
  char body[20];
  int count;
} Message;

// led
uint8_t disBuff[2 + 5 * 5 * 3];
bool blinkState = true;

// wifi config
const char* wifiSSID = WIFI_SSID;
const char* wifiPass = WIFI_PASSWORD;

// queue
QueueHandle_t queue;
int queueSize = 10;

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
 
  queue = xQueueCreate( queueSize, sizeof( int ) );
  if(queue == NULL){
    Serial.println("Error creating the queue");
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
  StaticJsonDocument<254> pubjd;  
  unsigned int stateInt = 0;
  while (1) {
    //vTaskDelay(10 / portTICK_PERIOD_MS);
    
    Serial.println("waiting for a message (subscribe)");
    PubSubClient* subclient = PubNub.subscribe(commandChan);
    if (!subclient) {
      Serial.println("subscription error");
      delay(1000);
      return;
    }
    
    String           msg;
    SubscribeCracker ritz(subclient);
    while (!ritz.finished()) {
      ritz.get(msg);
      if (msg.length() > 0) {
        Serial.print("Received: ");
        Serial.println(msg);
        
        pubjd.clear();
        deserializeJson(pubjd, msg);
  
        if (!pubjd.containsKey("state")) {
          Serial.println("state missing");
        } else {
          bool state = pubjd["state"].as<bool>();
          Serial.print("got state: "); Serial.println(state);
  
          if (state) {
            stateInt = 1;
          } else {
            stateInt = 0;
          }
          
          xQueueSend(queue, &stateInt, 0);
        }
      }
    }
  
    subclient->stop();
  }
}
 
void mainTask( void * parameter)
{
  int element;
  while (1) {
    loop_hardware();
    
    // See if there's a message in the queue (do not block)
    if (xQueueReceive(queue, (void *)&element, 10) == pdTRUE) {
      Serial.print(element);
      Serial.print("|");
    }
  }
}
