## 220V controlled ESP8266 IR Ceiling Fan transmitter 
We recently bought a  "Lyndby" (vanilla brand) ceiling Fan for our new home with an embedded light. It is controlled by an IR remote, normally sitting on a night stand next to our bed.
The problem is that the fan is mounted above our bed, where normally a bedroom ceiling light fixture sits which is wired to the standard on-off light switch next to the door.
It would of course be possible to mount a second lamp, but that would be a bit odd.

So I decided to craft an IR trasmitter controlled by the regular 220V light switch, that mimics the (light) signal of the remote.

The approach I took;  a small box with an esp8266 (Wemos D1 mini), IR LED, and optocoupled 220 detection.
I used the [IRremoteESP8266 library](https://github.com/crankyoldgit/IRremoteESP8266) to analyse and send signals.

This is how the final result looks
![This is how it looks](/images/Main%20pic.jpg)  

This is how it works
https://github.com/MichielfromNL/ESP8266IRFancontroller/blob/main/images/How%20it%20works.mp4

The schematics
![The schematics](/images/Schematics%20IR%20Fan.jpg

The prototype
![The Prototype](/images/Prototype.jpeg)

The final PCB
![The PCB](/images/PCB.jpg)


## Some details about the code & project.

### Photo resistor
On the PCB you probably notice a photoresistor that was not in the schenatics. The photoresistor is serially wired to a 10K resistor (betwen gnd and vcc) and connected to A0.
This is because I wanted to  find out if I can detect if the light is (already) on or off, but that does not work. The daylight interferes way to much.

### IR commands
It took some time to figure out the IR command codes. 
The FAN unit appears to be from Westinghouse, but there are quite a few variants. 
The basic protocol turns out to be "Symphony", but the sequences are a bit different from usual: a standard single bit sequence for each command.

The IR analyzer (breadboard prototype) reported: 
```Protocol  : SYMPHONY
Codes      :  (12 bits)
uint16_t rawData[95] = {
1254, 432,  1254, 432,  408, 1276,  408, 1256,  428, 1276,  410, 1276,  408, 1276,  408, 1278,  408, 1278,  406, 1276,  410, 1276,  408,
7940,  1254, 432,  1254, 432,  408, 1276,  408, 1276,  408, 1278,  1252, 434,  1254, 432,  1254, 430,  1254, 432,  1254, 434,  1252, 432,  1252, 
7096,  1254, 432,  1254, 432,  408, 1278,  408, 1276,  408, 1276,  410, 1256,  428, 1276,  408, 1276,  1254, 432,  410, 1274,  408, 1276,  408, 
7940,  1254, 434,  1252, 432,  408, 1276,  410, 1276,  408, 1278,  408, 1278,  408, 1276,  408, 1278,  1252, 434,  406, 1276,  408, 1276,  408};  // SYMPHONY C00
```

A sequence of approx 12xx usecs then 4xx usecs is a 1, 4xx followed by 12xx is a 0.  The 7xxx  usecs is pause
Hence, the (on/off) sequence above is:  0xC00 , 0xC7F, 0xC08 ; each 12 bits.  the last command is  repeated 3 x
By repeating this for each remote button, I found the following commands:

![image](https://user-images.githubusercontent.com/80706499/137891934-c97163ce-37df-450b-a9c0-77ea92459cf7.png)

Each command must be repeated 3 times. The dimmer, of course, as many times as needed to set the light at the desired level (if you want to use that via the Telnet interface)

The proper way to send these using the IR library turned out to be:

```
//  Send the Fan IR command
void sendCmd(IRcommand cmd, int repeat = 3) {
    irsend.sendSymphony(0xC00,12,1);
    irsend.sendSymphony(0xC7F,12,1);
    irsend.sendSymphony(cmd,12,repeat);
}
```
# Wemos D1 configuration
The Arduino IDE Wemos D1 config for this sketch is:
Flash size:  4MB, 1MB OTA, 115200,  Use the OTA port to flash
Note: the initial upload requires an USB connection. That must be done with the 3v3 jumper disconnected, to prevent a clash between the onboard power from USB and the 3v3 from the power supply. *NEVER POWER THE PCB and the USB at the same time *

## Telnet
For experimenting, I added a telnet interface using the [ESP Telnet](https://github.com/LennartHennigs/ESPTelnet) library from Lennart Hennigs.
Typing a question mark shows the possible commands
At some time, using that interface you can connect it to home automation, sending commands through that interface.
Of course, this is pretty unsafe. NO UID /PWD or the like, so never ever make this accessible from the internet

Have fun !

