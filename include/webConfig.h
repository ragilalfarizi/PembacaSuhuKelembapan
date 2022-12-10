#ifdef ESP32
#include <WebServer.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <HTTPUpdateServer.h>


WebServer server(80);
HTTPUpdateServer httpUpdater;

#else
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

#endif
#include "sendHTML.h"
#include "bacadht.h"

/*Initialize server & httpUpdater object*/

uint32_t chipId = 0;
uint32_t chipModel = 0;
const int led = 26;

void handleRoot() {
  String message = "Selamat datang di ESP8266!\n\n";
  message += "Nama : Yumna Salma Nabila \nNIM : H1A020002 \nUniversitas Jenderal Soedirman\n";
  message += "Chip Model = ";
  message += String(ESP.getFlashChipMode()).c_str();
  message += "\nChipID = ";
  message += String(chipId).c_str();
  message += "\nMyIP Address = ";
  message += String(WiFi.localIP().toString().c_str());
  //server.send(200, "text/plain", message);
  String IP = WiFi.localIP().toString().c_str();
  //chipModel = String(ESP.getChipModel()).c_str(); 
  server.send(200, "text/html", SendHTML(Temperature, Humidity, IP, chipId));
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URL: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}