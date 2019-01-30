//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a another network and broadcast message from a webpage to the edges of the mesh network.
// This sketch can be extended further using all the abilities of the AsyncWebserver library (WS, events, ...)
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
// for more details about my version
// https://gitlab.com/Assassynv__V/painlessMesh
// and for more details about the AsyncWebserver library
// https://github.com/me-no-dev/ESPAsyncWebServer
//************************************************************
#define ARDUINOJSON_USE_LONG_LONG 1 //prevents parseJason from causing exception
#include <ArduinoJson.h>
#include "IPAddress.h"
#include "painlessMesh.h"
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#ifdef ESP8266
#include "Hash.h"
#include <ESPAsyncTCP.h>
#else
#include <AsyncTCP.h>
#endif
//#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "whateveryoulike"
#define   MESH_PASSWORD   "somethingsneeky" //must be 8 characters or more
#define   MESH_PORT       5555

#define HOSTNAME "HOST_SERVER"
Scheduler     userScheduler; // to control your personal task

// Prototype
void receivedCallback( const uint32_t &from, const String &msg );
IPAddress getlocalIP();

painlessMesh  mesh;
//AsyncWebServer server(80);
ESP8266WebServer server(80);

IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);

void handleBody(){
    server.send(200, "text/html", "<form>Text to Broadcast<br><input type='text' name='BROADCAST'><br><br><input type='submit' value='Submit'></form>");
    if (server.hasArg("BROADCAST")){
      String msg = server.arg("BROADCAST");
      mesh.sendBroadcast(msg);
    }
}

void setup() {
  Serial.begin(115200);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //reset saved settings
  //wifiManager.resetSettings();

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(HOSTNAME);

String LAN_ID = WiFi.SSID();
String LAN_PASSWORD = WiFi.psk();
int LAN_CHANNEL = WiFi.channel();
Serial.print("Channel: ");
Serial.println(LAN_CHANNEL);

  WiFi.persistent(false);
  WiFi.disconnect();

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, LAN_CHANNEL );
  //mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, WiFi.channel() );
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(LAN_ID, LAN_PASSWORD);
  mesh.setHostname(HOSTNAME);
  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

  //Async webserver
  server.on("/", handleBody);
  
  server.begin();

}

void loop() {
  server.handleClient();
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("My IP is " + myIP.toString());
  }
}


void receivedCallback( const uint32_t &from, const String &msg ) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
