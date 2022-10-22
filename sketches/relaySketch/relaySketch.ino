/*
   ESP8266 relay control interface with ESP NOW wireless trigger

   When a message is received from a paired module using ESP NOW,
   a relay is momentarily turned on and off.
   
   Target Hardware:  ESP8266 WeMos D1 Mini and relay control pcb:
                     https://github.com/GadgetReboot/Remote_Relay
                   
   Tested with Arduino IDE 1.8.13
               ESP8266 board file 3.0.2
               
   Gadget Reboot
   https://www.youtube.com/gadgetreboot

*/

#include <ESP8266WiFi.h>
#include <espnow.h>

#define   relayPin        D5            // relay control output pin   high = on low = off
#define   relayOFF        0             // 0 = off 
#define   relayON         1             // 1 = on

int relayTriggerPeriod = 500;           // how long (mS) to turn on relay when triggered
int triggerTimeoutPeriod = 1000;        // how long (mS) before allowing another trigger (re-trigger lockout period)

// Replace with MAC Address of the other module being communicated with
// this sketch is in the Relay board so it talks to the Current Transformer board at this MAC address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

boolean incomingEventTrigger = false;    // set to true when a message is received with a trigger request

//structure to receive data into
//must match the sender data structure
typedef struct msg1 {
  boolean eventTriggered;
} msg1;

// create a struct msg to receive incoming data
msg1 msgToRelay;

void setup() {
  
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...waiting for trigger messages.\r\n\r\n");

  digitalWrite(LED_BUILTIN, HIGH);  // turn off esp8266 onboard led
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(relayPin, LOW);      // turn off relay
  pinMode(relayPin, OUTPUT);

  // start configuring wireless interface
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // register callback function for when data received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  processEvents();      // evaluate whether an event is triggered and if so, control relay
}


void processEvents() {

  static boolean currentlyTriggered = false;                  // track if an event trigger is in progress
  static boolean relayIsOn = false;                           // track if relay is on or off
  static unsigned long triggerTimer = millis();               // track relay on-time when triggered
  static unsigned long triggerExpireTimer = millis();         // track how long since trigger began

  // when a new trigger occurs, start timers and turn relay on
  if (!currentlyTriggered && incomingEventTrigger) {
    currentlyTriggered = true;
    triggerExpireTimer = millis();
    triggerTimer = millis();
    controlRelay(relayON);
    relayIsOn = true;
    Serial.println("New trigger starting...\r\nRelay turning on.");
  }


  // turn off relay when trigger has been active for declared relay on-period
  if (currentlyTriggered) {
    if ((unsigned long)(millis() - triggerTimer) > relayTriggerPeriod) {
      if (relayIsOn) {
        Serial.println("Relay turning off.");
      }
      controlRelay(relayOFF);
      relayIsOn = false;
    }
  }

  // cancel triggered state and allow new trigger events
  // when re-trigger lockout duration has passed
  if (currentlyTriggered) {
    if ((unsigned long)(millis() - triggerExpireTimer) > triggerTimeoutPeriod) {
      currentlyTriggered = false;
      incomingEventTrigger = false;
      Serial.println("Trigger period ending...");
    }
  }
}

void controlRelay(int state) {             // turn relay on or off based on "state"
  digitalWrite(LED_BUILTIN, !state);       // WeMOS LED is active low
  digitalWrite(relayPin, state);           // relay is active high
}

// callback function for when data received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&msgToRelay, incomingData, sizeof(msgToRelay));    // copy received data into the struct data variable(s)

  static uint16_t msgCounter = 0;                           // count the number of received messages
  msgCounter++;

  incomingEventTrigger = msgToRelay.eventTriggered;         // has a trigger request been received?

  Serial.print("\r\nIncoming msg #" + String(msgCounter));
  Serial.print("  Bytes received: ");
  Serial.println(len);

  Serial.print("eventTriggered: ");
  Serial.println(incomingEventTrigger ? "true" : "false");
}
