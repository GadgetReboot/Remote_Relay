Sketches that go into ESP8266 for the sending and receiving modules (current transformer and relay control modules)<BR><BR>
The Relay sketch goes into the Remote Relay pcb and <BR>
The Current Transformer sketch goes into this pcb:  https://github.com/GadgetReboot/WiFi_Current_Transformer<BR><BR>
The modules communicate wirelessly using ESP-NOW, and the sketches must be modified to enter the mac address of the ESP8266 on the other board they are communicating with.<BR>
Eg. The relay board sketch should contain the mac address of the current transformer board's ESP8266 and vice versa.<BR><BR>
The mac address of each module can be found by running the sketch that prints out the mac address on the serial monitor, then enter the appropriate mac address in the sketches.
