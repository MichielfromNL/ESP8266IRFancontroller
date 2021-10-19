/* 
 *  IR-Send-Fan
 *  
 *  Michiel Steltman 2021
 *  
 *  Send IR codes Light on/off to our ceiling FAN 
 *  sending codes as captured and analyzed by IRReceive V2. 
 *  Protocol is Symphony
 *  
 *  Input:  D6, opto ocoupler. If state changes, we send the IR on/off sequence 
 *  We do the update via OTA
 *  
 *  There is a photo resistor at A0. We dont use it yert, perhaps later,
 *  
 */
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPTelnet.h>

const int IrLed = D2; // Wemos D1 GPIO 4 (D2), IR led output
const int VSens = D6; // Wemos D1 GPIO 12 (D6) , Optocoupler 220V sensor input.
const int Lsens = A0; // Wemos D1 ADC0 A0 analogue in for light sensor

const char * ssid = "wifiSSID";
const char * password = "YourWifiPASS";
const char * Hostname = "ESPFanControl";

IRsend    irsend(IrLed);  // Setup the IR lib with the LED output .
ESPTelnet telnet;
bool      connected = false;

// This is the on-off sequence captured by IRrecvDumpV2.ino
/* 
uint16_t rawOnOff[95] = {
1274, 434,  1250, 434,  406, 1280,  404, 1258,  428, 1278,  404, 1280,  404, 1278,  406, 1282,  404, 1278,  406, 1280,  404, 1280,  406, 
7940,  
1252, 434,  1250, 436,  406, 1278,  406, 1278,  406, 1280,  1250, 434,  1250, 436,  1250, 436,  1250, 434,  1250, 434,  1250, 436,  1250, 
7098,  
1252, 436,  1250, 436,  404, 1278,  406, 1278,  406, 1280,  406, 1280,  404, 1278,  406, 1280,  1252, 434,  406, 1280,  404, 1280,  404,
7942,  
1252, 434,  1250, 434,  406, 1278,  406, 1278,  406, 1280,  404, 1278,  406, 1280,  406, 1280,  1250, 434,  406, 1280,  404, 1256,  428};  // SYMPHONY C00 C7f C08 C08
*/
// Example captured raw on.off : 
// 1100 0000 0000  (C00)
// 1100 0111 1111  (C7F)
// 1100 0000 1000  (C08) (repteated 3 x )
//
//  = 12 bits, all commands start with 0xC00 , 0xC7F
//
//  cmds are:
//  On/Off    0xC08
//  Dimmer    0xC20
 // Fan High  0xC01
//  Fan Med   0xC04
//  Fan Low   0xC43
//  Fan Off   0xC10
//
// 
enum IRcommand {
  light     = 0xC08,
  fanhigh   = 0xC01,
  fanmedium = 0xC04,
  fanlow    = 0xC43,
  fanoff    = 0xC10,
  dimmer    = 0xC20
};

//
// Setup wifi and OTA
//
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(50);

  irsend.begin();
  pinMode(VSens, INPUT);
  
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
    // keep connection alive
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(Hostname);

  // set code to execute on OTA connect
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
    telnet.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    telnet.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char s[128];
    sprintf(s,"Progress: %u%%\r", (progress / (total / 100)));
    Serial.println(String(s));
    telnet.println(String(s));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    const char *s;
    char errstr[128];
    if (error == OTA_AUTH_ERROR) s = "Auth Failed";
    else if (error == OTA_BEGIN_ERROR) s = "Begin Failed";
    else if (error == OTA_CONNECT_ERROR) s = "Connect Failed";
    else if (error == OTA_RECEIVE_ERROR) s = "Receive Failed";
    else if (error == OTA_END_ERROR) s = "End Failed";
    sprintf(errstr,"Error[%u]: %s ", error , s);
    
    Serial.println(String(s));
    telnet.println(String(s));
  });

  Serial.print(Hostname);
  Serial.println(" Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  setupTelnet();
  ArduinoOTA.begin();
}

//  Send an IR command
void sendCmd(IRcommand cmd, int repeat = 3) {
    irsend.sendSymphony(0xC00, 12,1);
    irsend.sendSymphony(0xC7F, 12,1);
    irsend.sendSymphony(cmd, 12,repeat);
}

//
//  Detect state change and send 
//  Later: measure light intensity. (add circuit first)

int lastState = 1; // (off)
void loop() {
  
  if (digitalRead(VSens) != lastState) 
    // debounce, 
    delay(50);
  if (digitalRead(VSens) != lastState) {
    //Serial.println("State change");
    lastState = digitalRead(VSens);
    // Serial.println("Light switch = " + lastState ? "On" : "Off");
    Serial.println("Send IR Symphony light on/off");
    telnet.println("Send IR Symphony light on/off");
    sendCmd(light);
  }

  ArduinoOTA.handle();
  delay (500);
  // check Wifi status and reconnect if necessary
  telnet.loop();
}

//
bool setupTelnet() {  
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);
  telnet.onInputReceived(onTelnetInput);

  Serial.print("- Telnet: ");
  if (telnet.begin()) {
    Serial.println("Telnet server on port 23 ");
    return true;
  } else {
    Serial.println("error.");
    return false;
  }
}

/* ------------------------------------------------- */

// (optional) callback functions for telnet events
void onTelnetConnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" connected");
  telnet.println("\nWelcome " + telnet.getIP());
  //telnet.println("(Use ^] + q  to disconnect.)");
  connected = true;
}

void onTelnetDisconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" disconnected");
  connected = false;
}

void onTelnetReconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" reconnected");
  connected = true;
}

void onTelnetConnectionAttempt(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" tried to connected");
}

// handle input
//
void onTelnetInput(String str) {
   // checks for a manual command
  
  switch (str.c_str()[0]) {
    case '?' : 
      telnet.println("+ = light on/off");
      telnet.println("d = Dimmer");
      telnet.println("3 = Fan High");
      telnet.println("2 = Fan Medium");
      telnet.println("1 = Fan Low");
      telnet.println("0 = Fan Off");
      telnet.print("Light switch = "); telnet.println(digitalRead(VSens) ? "Off" : "On");
      telnet.println("Photo sensor value (0-1024) = " + String(analogRead(Lsens)));
      break;
    case '+':
      sendCmd(light);
      telnet.println("Light On/Off");
      break;
    case '3':
      sendCmd(fanhigh);
      telnet.println("Fan High");
      break;
    case '2':
      sendCmd(fanmedium);
      telnet.println("Fan Medium");
      break;
    case '1':
      sendCmd(fanlow);
      telnet.println("Fan Low");
      break;
    case '0':
      sendCmd(fanoff);
      telnet.println("Fan Off");
      break;
    case 'd':
      sendCmd(dimmer,100);
      telnet.println("Dimmer");
      break;
    default:
      telnet.println("Invalid command");
  }
}
