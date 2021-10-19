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

The schematics:
![The schematics](/images/Schematics%20IR%20Fan.jpg)

The prototype
![The Prototype](/images/Prototype.jpeg)

The final PCB
![The PCB](/images/PCB.jpg)


## Some details about the code & project.
It took some time to figure out the IR command codes.

The IR analyzer reported: 
`Protocol  : SYMPHONY
Codes      :  (12 bits)
uint16_t rawData[95] = {1254, 432,  1254, 432,  408, 1276,  408, 1256,  428, 1276,  410, 1276,  408, 1276,  408, 1278,  408, 1278,  406, 1276,  410, 1276,  408,
7940,  1254, 432,  1254, 432,  408, 1276,  408, 1276,  408, 1278,  1252, 434,  1254, 432,  1254, 430,  1254, 432,  1254, 434,  1252, 432,  1252, 
7096,  1254, 432,  1254, 432,  408, 1278,  408, 1276,  408, 1276,  410, 1256,  428, 1276,  408, 1276,  1254, 432,  410, 1274,  408, 1276,  408, 
7940,  1254, 434,  1252, 432,  408, 1276,  410, 1276,  408, 1278,  408, 1278,  408, 1276,  408, 1278,  1252, 434,  406, 1276,  408, 1276,  408};  // SYMPHONY C00`

A sequence of approx 1250 usecs then 403 usecs is a 1, 40x followed by 12xx is a 0.  A 7940 is pause
Hence, the sequence is:  0xC00 , 0xC7F, 0xC08 ; each 12 bits.  the last command is  repeated 3 x )
Analyzing this, I found the following commands:

All codes start with 0xC00 , 0xC7F, and then:
![image](https://user-images.githubusercontent.com/80706499/137891934-c97163ce-37df-450b-a9c0-77ea92459cf7.png)

Each command must be repeated 3 times. The dimmer of course as many times as needed to set the light at the desired level

The proper way to send these using the IR library turned out to be:
`//  Send an IR command
void sendCmd(IRcommand cmd, int repeat = 3) {
    irsend.sendSymphony(0xC00, 12,1);
    irsend.sendSymphony(0xC7F, 12,1);
    irsend.sendSymphony(cmd, 12,repeat);
}`


## Telnet
For experimenting, I added a telnet interface, using the [ESP Telnet](https://github.com/LennartHennigs/ESPTelnet) library from Lennart Hennigs.
At some time, using that interface you can connect it to home automation, sending commands through that interface.
Of course, this is pretty unsafe. NO UID /PWD or the like, so never ever make this accessible from the internet

Have fun !


<!---
MichielfromNL/MichielfromNL is a ✨ special ✨ repository because its `README.md` (this file) appears on your GitHub profile.
You can click the Preview link to take a look at your changes.
--->
