#include <Arduino.h>
#include <BleCombo.h>

#include "WiFi.h"
#include <WiFiUdp.h>
#include <WiFiServer.h>

//#define DEBUG

// tcp settings
#define TCP
#define TCPPORT 8088

// udp settings
#define UDP
#define UDPPORT 8888

#ifdef UDP
WiFiUDP udp;
#endif

#ifdef TCP
WiFiServer server(TCPPORT);
WiFiClient client;
#endif

char packetBuffer[255];

// wlan settings
const char* ssid = "key";
const char* pass = "virtualkey";

bool btConnected;
bool serialecho = false;

String srialcommand;

// 
// ParseCommand
// 
bool ParseCommand(String cmd) {
  String part[10];
  int partcounter=0;
  
  #ifdef DEBUG
  Serial.printf("length: %d\n",cmd.length());
  #endif

  for(int i=0;i<cmd.length();i++) {
    if(cmd[i]=='\n'||cmd[i]=='\r'||cmd[i]=='\0') {
      #ifdef DEBUG
      Serial.printf("break:%d at pos: %d\n",int(cmd[i]),i );
      #endif
      break;
    }

    if(cmd[i]==' ') {
      partcounter++;
      #ifdef DEBUG
      Serial.printf("counter:%d\n", partcounter );
      #endif
      if(partcounter>=10) {
        return false;
      }
    } else {
      part[partcounter] += cmd[i];
    }
  }

  #ifdef DEBUG
  Serial.printf("part 1:[%s]\n", part[0] );
  #endif

  if(btConnected) {  
    if(part[0]=="pri") {
      Keyboard.print(part[1]);
      return true;
    } else if(part[0]=="wri") { // Write a character
      uint temp = part[1].toInt();
      Keyboard.write(temp);
      return true;
    } else if(part[0]=="raw") { // Press a key
      uint temp = part[1].toInt();
      Keyboard.pressraw(temp);  
      return true;
    } else if(part[0]=="mod") { // Press a modification key
      uint temp = part[1].toInt();
      Keyboard.pressmod(temp);
      return true;
    } else if(part[0]=="pre") { // Press a key
      uint temp = part[1].toInt();
      Keyboard.press(temp);  
      return true;
    } else if(part[0]=="rel") { // Release all keys
      Keyboard.releaseAll();    
      return true;
    } else if(part[0]=="rer") { // Release a key (rawcode)
      signed char temp = part[1].toInt();
      Keyboard.releaseraw(temp);   
      return true;
    } else if(part[0]=="mov") { // Move mouse
      signed char tempx = part[1].toInt();
      signed char tempy = part[2].toInt();
      signed char tempw = part[3].toInt();
      signed char tempwh = part[4].toInt();
      Mouse.move(tempx, tempy, tempw, tempwh);     
      return true;
    } else if(part[0]=="cli") { // Click mouse
      uint temp = part[1].toInt();
      Mouse.click(temp);      
      return true;
    } else if(part[0]=="mpr") { // Press mouse button
      uint temp = part[1].toInt();
      Mouse.press(temp);    
      return true;
    } else if(part[0]=="mre") { // Release mouse button
      uint temp = part[1].toInt();
      Mouse.release(temp);
      return true;
    } else if(part[0]=="sta") {  // Status of the blconnection
      return true;
    } else if(part[0]=="eco") {  // Echo serial input
      if(serialecho) {
        serialecho = false;
      } else {
        serialecho = true;
      } 
      return true;
    }
  } else {
    if(part[0]=="deb") { // Debug info
      Serial.println(part[1]);
      for(int i=0;i<part[1].length();i++) {
        int ascii = part[1][i];
        Serial.println(ascii);
      }
      return true;
    } else if(part[0]=="sta") {  // Status of the blconnection
      return false;
    } else if(part[0]=="eco") {  // Echo serial input
      if(serialecho) {
        serialecho = false;
      } else {
        serialecho = true;
      } 
      return true;
    }
    return false;
  }
  return false;
}

//
// setup
//

void setup() {
  WiFi.softAP(ssid, pass);
  Serial.begin(115200);
  Serial.println("starting");
  Keyboard.begin();
  Mouse.begin();
  #ifdef TCP
  server.begin();
  #endif
  #ifdef UDP
  udp.begin(UDPPORT);
  #endif
}

//
// loop
// 

void loop() {
  //
  // Handle Terminal
  //

  if(Serial.available()>0) {
    char c = Serial.read();
    if (c == '\r'){
      // Jump over
    } else if (c == '\n') {
      if(serialecho) {
        Serial.write('\n');
      }
      if( ParseCommand(srialcommand) ){
        Serial.println("ok");
      } else {
        Serial.println("nok");
      }
      srialcommand = "";
    } else {
      if(serialecho) {
        Serial.write(c);
      }
      srialcommand += c;
    }
  }

  //
  // Handle bluetooth connection state
  //

  if(Keyboard.isConnected()) {
    if(!btConnected) {
      Serial.println("con"); // bluetooth connected
      #ifdef UDP
      if( udp.beginPacket(udp.remoteIP(), udp.remotePort()) ) {
        udp.printf("con");
        udp.endPacket();
      }
      #endif
    }
    btConnected = true;
  } else {
    if(btConnected) {
      Serial.println("dis"); // bluetooth disconnected
      #ifdef UDP
      if( udp.beginPacket(udp.remoteIP(), udp.remotePort()) ) {
        udp.printf("dis");
        udp.endPacket();
      }
      #endif
    }
    btConnected = false;
  }

  
  //
  // Handle UDP communication
  //
  #ifdef UDP

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, 255);
    #ifdef DEBUG
    Serial.printf("Len: %d\n",len);
    #endif
    if (len > 0) {
      packetBuffer[len] = '\0';
    }
    #ifdef DEBUG
    Serial.println("["+String(packetBuffer)+"]");
    #endif
    if(ParseCommand( String(packetBuffer))){
      if(udp.beginPacket(udp.remoteIP(), udp.remotePort())) {
        udp.printf("ok");
        udp.endPacket();
      } else {
        Serial.println("Error: UDP packet could not send. (ok)");
      }
    } else {
      if(udp.beginPacket(udp.remoteIP(), udp.remotePort())) {
        udp.printf("nok");
        udp.endPacket();
      } else {
        Serial.println("Error: UDP packet could not send. (nok)");
      }
    }
  }

  #endif
  
  //
  // Handle TCP connection
  //
  #ifdef TCP
  if(!client) {
    client = server.available();
  }
  if(client.connected()) {
    if (client.available()) {
      String data = client.readString();
      if(ParseCommand( String(packetBuffer))){
        client.printf("ok");
      } else {
        client.printf("nok");
      }
    }
  }
  #endif
}

