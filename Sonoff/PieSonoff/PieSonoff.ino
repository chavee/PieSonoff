#include <AuthClient.h>
#include <MicroGear.h>
#include <MQTTClient.h>
#include <SHA1.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <MicroGear.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define RELAYPIN 12
#define LEDPIN 13
#define EEPROM_STATE_ADDRESS 128

#define APPID   "deerdev"
#define KEY     "JMi3V8gkiq80s1N"
#define SECRET  "czWPHQlMYDyW0Pml2wRlpz8uBfskF5"
#define ALIAS   "piesonoff"

WiFiClient client;
AuthClient *authclient;

char state = 0;

MicroGear microgear(client);

void sendState() {
  if (state==0)
    microgear.publish("/piesonoff/state","0");
  else
    microgear.publish("/piesonoff/state","1");
}

void updateIO() {
  if (state == 1) {
    digitalWrite(RELAYPIN, HIGH);
    digitalWrite(LEDPIN, LOW);
  }
  else {
    state = 0;
    digitalWrite(RELAYPIN, LOW);
    digitalWrite(LEDPIN, HIGH);
  }
}

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  char m = *(char *)msg;
  
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
  
  if (m == '0' || m == '1') {
    state = m-'0';
    EEPROM.write(EEPROM_STATE_ADDRESS, state);
    EEPROM.commit();
    updateIO();
  }
  if (m == '0' || m == '1' || m == '?') sendState();
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  sendState();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    pinMode(RELAYPIN, OUTPUT);
    pinMode(LEDPIN, OUTPUT);

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
