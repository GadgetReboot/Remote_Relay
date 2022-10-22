/*
   SCT013 Current Transformer interface with ESP NOW wireless trigger
   Continuously monitors the rms current and when a threshold is reached,
   triggers an event to alert another ESP8266, which then activates a relay.

   Target Hardware:  ESP8266 WeMos D1 Mini and op amp circuit pcb:
                     https://github.com/GadgetReboot/WiFi_Current_Transformer

   Libraries used: EmonLib
                   ESPNow

   Tested with Arduino IDE 1.8.13
               ESP8266 board file 3.0.2

   Gadget Reboot
   https://www.youtube.com/gadgetreboot

*/

#include <ESP8266WiFi.h>
#include <espnow.h>

// Replace with MAC Address of the other module being communicated with
// this sketch is in the "Current Transformer" board so it talks to the Relay board at this MAC address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};



//--------current transformer items-------------
#include <EmonLib.h>

EnergyMonitor emon1;                    // create an energy monitor instance

#define triggerOut   4                  // pin D2 (GPIO4) is trigger out
#define rmsThreshold 300                // threshold rms current (mA) to trigger on
#define emonCal      4.92               // experimentally determined calibration for current readings
#define adcPin       A0                 // current transformer input
//#define emonSamples 3290              // number of samples to read from adc for rms current measurement (10 cycles)
#define emonSamples  1645               // number of samples to read from adc for rms current measurement (5 cycles)

int currentRms = 0;                     // last measured rms current value in mA

/*     How to choose value for emonSamples:

       AC frequency = 60 Hz
       Sample 10 cycles of AC waveform for rms measurement
       Required sample time for 10 cycles of 60 Hz AC
       (1 / 60cycles per second) * 10 cycles = 166.67 mS

       Running a test sketch with emonSamples set to 1000
       to measure time for esp8266 to take 1000 samples on ADC,
       the time to read the adc for 1000 samples was 50.66 mS

       Required number of ADC samples to measure 10 cycles of 60 Hz AC
       = 166.67 mS * (1000 samples / 50.66 mS)
       = 3290 samples

       For faster response eg detecting a fast doorbell press ~100mS,
       use half the samples for 5 cycles
*/

//-------------------------------


//Structure example to send data
//Must match the receiver structure
typedef struct msg1 {
  boolean eventTriggered;
} msg1;

// create a struct msg for transmission to relay board
msg1 msgToRelay;

void setup() {

  Serial.begin(115200);
  Serial.println();

  digitalWrite(triggerOut, 0);      // turn off current trigger gpio output
  pinMode(triggerOut, OUTPUT);

  emon1.current(adcPin, emonCal);   // initialize current reading on adc pin with a calibration adjustment

  // take some dummy current measurements to let numbers settle
  // (initial few readings are way high)
  for (int i = 0; i < 10; i++) {
    float rmsVal = emon1.calcIrms(emonSamples);     // calculated rms value of adc voltage readings
    Serial.print("Startup current (mA): ");
    Serial.println(rmsVal * 1000);
  }

  // start configuring WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // register a callback function for sent data
  // also provides confirmation of send success/failure
  esp_now_register_send_cb(OnDataSent);

  // register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // register a callback function for received data
  // esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

  float rmsVal_mA = emon1.calcIrms(emonSamples) * 1000;                           // calculated rms value of adc voltage
  boolean triggerStatus = (rmsVal_mA >= rmsThreshold ? true : false);

  int pingInterval = 500;                                                         // print current reading at interval
  static unsigned long previousMillis = millis();

  // periodic ping message
  if (millis() - previousMillis >= pingInterval) {
    previousMillis = millis();
    Serial.print("\r\nRMS Current (mA): ");
    Serial.println(rmsVal_mA);
    Serial.print("triggerStatus: ");
    Serial.println(triggerStatus ? "true" : "false");
  }

  // when a current threshold is reached, send a relay control
  // message to the other module using ESP-NOW
  if (triggerStatus) {
    msgToRelay.eventTriggered = triggerStatus;                                    // assign message data
    esp_now_send(broadcastAddress, (uint8_t *) &msgToRelay, sizeof(msgToRelay));  // send message to other module

    Serial.print("\r\nTriggered with RMS Current (mA): ");
    Serial.println(rmsVal_mA);

    Serial.print("triggerStatus: ");
    Serial.println(triggerStatus ? "true" : "false");

    delay(100);  // if back to back messages are sent too fast they may fail delivery
  }
}

// Callback function for when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}
