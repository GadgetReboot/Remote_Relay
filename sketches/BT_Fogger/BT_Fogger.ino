/*
   ESP8266 relay control interface with bluetooth wireless trigger

   When a message is received from a bluetooth module connected to the uart,
   a relay is momentarily turned on and off.
   In this demo, a fog machine's push button wires are connected to the
   relay so fog can be controlled via bluetooth.
   
   Target Hardware:  ESP8266 WeMos D1 Mini and relay control pcb:
                     https://github.com/GadgetReboot/Remote_Relay
                   
   Tested with Arduino IDE 1.8.13
               ESP8266 board file 3.0.2
               
   Gadget Reboot
   https://www.youtube.com/gadgetreboot

*/


#define   relayPin        D5          // relay control output pin   high = on low = off

String btData = "";                   // bluetooth incoming data gets appended to this string and used as commands

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);    // turn off esp8266 onboard led
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(relayPin, LOW);        // turn off relay
  pinMode(relayPin, OUTPUT);

  Serial.begin(9600);                 // bluetooth module data rate
  Serial.swap();                      // reconfigure uart pins for tx = D8/GPIO15 and Rx = D7/GPIO13
}

void loop() {
  while (Serial.available() > 0) {     // read characters from bluetooth uart and append to string
    char data = (char) Serial.read();
    if (data == '#') {                 // when # arrives, parse the string for commands
      if (btData == "fog0") {          // command "fog0" means turn off fogger
        controlRelay(0);
      }
      else if (btData == "fog1") {     // command "fog1" means turn on fogger
        controlRelay(1);
      }
      btData = "";                     // reset command string when processed and start reading a new command
    }
    else {
      btData += data;                  // if received character was not "#" continue appending characters to string
    }
  } // end while
}  // end loop

void controlRelay(int state) {         // turn relay on or off based on "state"
  digitalWrite(LED_BUILTIN, !state);   // WeMOS LED is active low
  digitalWrite(relayPin, state);       // relay is active high
}
