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
https://github.com/MichielfromNL/ESP8266IRFancontroller/images/How%20it%20works.mp4

The schematics:
![The schematics](/images/Schematics%20IR%20Fan.jpg)

The prototype
![The Prototype](/images/Prototype.jpeg)

The final PCB
![The PCB](/images/PCB.jpeg)

Some details about the code.
It took some time to figure out the IR sequence.

Telnet


<!---
MichielfromNL/MichielfromNL is a ✨ special ✨ repository because its `README.md` (this file) appears on your GitHub profile.
You can click the Preview link to take a look at your changes.
--->
