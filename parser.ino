//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include "config.h"

TTGOClass *ttgo;

AsyncWebServer server(80);

const char* ssid = "FRITZ!Box 7590 NK";
const char* password = "AssB!bERl!n($)";

const char* PARAM_MESSAGE = "message";
const char* PARAM_TYPE = "type";
const char* PARAM_EVENTS = "events";
const char* PARAM_VALUES = "values";
const char* PARAM_TIMEFRAME = "timeframe";
const char* PARAM_OPERATIONS = "operations";
const char* PARAM_TIMESTAMP = "timestamp";

String siddhi;

WiFiClient client;

void notFound(AsyncWebServerRequest *request) {
  Serial.println("Not Found");
  request->send(404, "text/plain", "Not found");
}

void setTime(){

}

void setup() {

  Serial.begin(115200);
  Serial.println("Starting");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
    String type = request->getParam(PARAM_TYPE, true)->value();

    if(type == "siddhi"){
      //siddhi querry setzen
      String message = request->getParam(PARAM_TYPE, true)->value();
      String events = request->getParam(PARAM_EVENTS, true)->value();
      String values = request->getParam(PARAM_VALUES, true)->value();
      String timeframe = request->getParam(PARAM_TIMEFRAME, true)->value();
      String operations = request->getParam(PARAM_OPERATIONS, true)->value();
      Serial.println("Siddhi");

    }else if(type == "time"){
      setTime();

    }else{
      //event recieved
      String events = request->getParam(PARAM_EVENTS, true)->value();
      String values = request->getParam(PARAM_VALUES, true)->value();
      String timestamp = request->getParam(PARAM_TIMESTAMP, true)->value();
      Serial.println("Something else");

      if(){

      }


    }

    Serial.println("Message: " + message);
    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);
  server.begin();


  IPAddress server(192, 168, 178, 53);
  client.connect(server, 6666);
  client.connect(server, 6666);
  client.connect(server, 6666);
  client.connect(server, 6666);
  client.connect(server, 6666);
  client.connect(server, 6666);

  Serial.println("Ping");
  client.println("IBM;700;100");
}

void loop() {
}