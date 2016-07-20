#include <MicroGear.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define RELAYPIN 12
#define LEDPIN 13
#define EEPROM_STATE_ADDRESS 128

#define ONMSG  "on"
#define OFFMSG "off"
#define CHKMSG "?"
#define ALIAS   "piesonoff"

// Please visit netpie.io to get keys
#define APPID   <APPID>       // e.g. "MySwitch"
#define KEY     <APPKEY>      // e.g. "4DPanXKaSdb2VrT"
#define SECRET  <APPSECRET>   // e.g. "ZgrXbHsaVp7TI8xW5oEcAqvY3"

WiFiClient client;

char state = 0;

MicroGear microgear(client);

void sendState() {
  if (state==0)
    microgear.publish("/piesonoff/state",OFFMSG,true);
  else
    microgear.publish("/piesonoff/state",ONMSG,true);
}


void updateIO() {
  if (state == 1) {
    digitalWrite(RELAYPIN, HIGH);
    #ifdef LEDPIN
      digitalWrite(LEDPIN, LOW);
    #endif
  }
  else {
    state = 0;
    digitalWrite(RELAYPIN, LOW);
    #ifdef LEDPIN
      digitalWrite(LEDPIN, HIGH);
    #endif
  }
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char *m = (char *)msg;
  
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);

  if (strcmp(m,OFFMSG)==0) {
    state = 0;    
  }
  else if (strcmp(m,ONMSG)==0) {
    state = 1;
  }
  else if (strcmp(m,CHKMSG)==0) {
    sendState();
    return;
  }
  else {
    return;
  }

  EEPROM.write(EEPROM_STATE_ADDRESS, state);
  EEPROM.commit();
  updateIO();
  sendState();
  
  /*
  if (strcmp(m,OFFMSG)==0 || strcmp(m,ONMSG)==0) {
    if (st)
    
    state = m=='0'?0:1;
    EEPROM.write(EEPROM_STATE_ADDRESS, state);
    EEPROM.commit();
    updateIO();
  //if ( strcmp(m,OFFMSG)==0 || strcmp(m,ONMSG)==0 || strcmp(m,CHKMSG)==0 ) sendState();
  }
  */
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  sendState();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    pinMode(RELAYPIN, OUTPUT);
    #ifdef LEDPIN
      pinMode(LEDPIN, OUTPUT);
    #endif
    EEPROM.begin(512);
    state = EEPROM.read(EEPROM_STATE_ADDRESS)==1?1:0;
    updateIO();

    WiFiManager wifiManager;
    wifiManager.setTimeout(180);

    if(!wifiManager.autoConnect("PieSonoff")) {
      Serial.println("Failed to connect and hit timeout");
      delay(3000);
      ESP.reset();
      delay(5000);
    }
  
    microgear.on(MESSAGE,onMsghandler);
    microgear.on(CONNECTED,onConnected);

    microgear.init(KEY,SECRET,ALIAS);
    microgear.connect(APPID);
}

void loop() {
  if (microgear.connected()) {    
    microgear.loop();
  }
  else {
    Serial.println("connection lost, reconnect...");
    microgear.connect(APPID);
  }
}
