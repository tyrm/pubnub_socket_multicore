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
 
  xTaskCreate(
    subscriberTask,     /* Task function. */
    "subscriber",       /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */
 
  xTaskCreate(
    mainTask,     /* Task function. */
    "main",       /* String with name of task. */
    10000,            /* Stack size in words. */
    NULL,             /* Parameter passed as input of the task */
    1,                /* Priority of the task. */
    NULL);            /* Task handle. */
 
  // Delete "setup and loop" task
  vTaskDelete(NULL);
}
 
void loop() {
}
 
void subscriberTask( void * parameter )
{
  for( int i = 0;i<queueSize;i++ ){
    xQueueSend(queue, &i, portMAX_DELAY);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }

  vTaskDelete( NULL );
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
