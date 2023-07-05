#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include <String.h>
#include <IPAddress.h>

//you need this: https://github.com/shah253kt/SimpleStack
#include <SimpleStack.h>


//plan:
//      - stack of events and stack of operations
//      - handle these like the calculator
//      - periodically perform the check
//      - if all criteria are met send to destination


struct event{
  String name;
  String origin;
  int timestamp;
}

//Each device has a certain sensor, once an event from this device is registerd the timestamp is put into this struct
// This is needed if we want to check if the same sensor on two different devices
struct device{
  bool hasGyro;
  bool hasTouch;
  bool hasJoy;
  bool hasHumidity;
  bool hasMic;

  event gyro;
  event touch;
  event joy;
  event humidity;
  event mic;
}


// Replace with your network credentials
const char* ssid = "FRITZ!Box 7590 NK";
const char* password = "AssB!bERl!n($)";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

TTGOClass *ttgo;
bool rtcIrq = false;
char buf[128];

//IPs of the other devices
IPAddress watch1(192, 168, 0, 1);
IPAddress watch2(192, 168, 0, 2);
IPAddress pi1(192, 168, 0, 4);
IPAddress pi2(192, 168, 0, 5);

//Components for the siddhi querry
String events;
String values;
String timeframe;
String operations;
String origins;
String destination;

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->rtc->setDateTime(0, 0, 0, 0, 0, 0);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  server.on("/time", HTTP_POST, [](AsyncWebServerRequest *request){
    String time = request->getParam("timestamp", true)->value();
    Serial.println("Time set to: " + time);

    int timestamp = time.toInt();

    int years = (timestamp / 31536000);
    timestamp -= years * 3153600;
    int month = (timestamp / 2592000);
    timestamp -= month * 2629440;
    int day = (timestamp / 86400);
    timestamp -= day * 86400;
    int hour = (timestamp / 3600);
    timestamp -= hour * 3600;
    int minute = (timestamp / 60);
    timestamp -= minute * 60;
    int second = (timestamp / 1);
    timestamp -= second * 1;

    ttgo->rtc->setDateTime(years, month, day, hour, minute, second);

    request->send_P(200, "text/html", "time set");
  });

  server.on("/ip", HTTP_POST, [](AsyncWebServerRequest *request){
    String ip = request->getParam("ip", true)->value();
    String device = request->getParam("device", true)->value();
    Serial.println(device + " found on IP: " + ip);
    
    if(device.equals("watch1")){
      watch1.fromString(ip);
      Serial.print("Watch 1: ");
      Serial.println(watch1);

    }else if(device.equals("watch2")){
      watch2.fromString(ip);
      Serial.print("Watch 2: ");
      Serial.println(watch2);

    }else if(device.equals("pi1")){
      pi1.fromString(ip);
      Serial.print("Pi1: ");
      Serial.println(pi1);

    }else if(device.equals("pi2")){
      pi2.fromString(ip);
      Serial.print("Pi2: ");
      Serial.println(pi2);
    }
    request->send_P(200, "text/html", "ip set");
  });

  server.on("/ip/dump", HTTP_POST, [](AsyncWebServerRequest *request){

    Serial.println();
    Serial.print("Watch 1: ");
    Serial.println(watch1);
    Serial.print("Watch 2: ");
    Serial.println(watch2);
    Serial.print("Pi1: ");
    Serial.println(pi1);
    Serial.print("Pi2: ");
    Serial.println(pi2);

    request->send_P(200, "text/html", "ip dumped to Serial");
  });

  server.on("/event", HTTP_POST, [](AsyncWebServerRequest *request){
    String event = request->getParam("event", true)->value();
    String value = request->getParam("value", true)->value();
    String timestamp = request->getParam("timestamp", true)->value();

    Serial.println("New " + event + " event with value " + value + "recieved at " + timestamp);

    request->send_P(200, "text/html", "event recieved");
  });

  server.on("/siddhi", HTTP_POST, [](AsyncWebServerRequest *request){
    events = request->getParam("events", true)->value();
    values = request->getParam("values", true)->value();
    timeframe = request->getParam("timeframe", true)->value();
    operations = request->getParam("operations", true)->value();
    origins = request->getParam("origins", true)->value();
    destination = request->getParam("destination", true)->value();

    Serial.println("New Siddhi querry recived which uses the " + operations +
                  "-operation on " + events + " that have values above " + values +
                  " in a timeframe of " + timeframe);
    request->send_P(200, "text/html", "siddhi querry recieved");
  });
  // Start server
  server.begin();
}

void loop() {
  ttgo->tft->setTextColor(random(0xFFFF));
  ttgo->tft->drawString("T-Watch RTC",  50, 50, 4);

  ttgo->tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
  ttgo->tft->drawString(buf, 5, 118, 7);
  delay(1000);
}
