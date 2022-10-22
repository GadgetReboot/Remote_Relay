// run this sketch in ESP8266 to have it print out the mac address

#include <ESP8266WiFi.h>
 
void setup(){
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.print("\r\n Mac address: ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}
