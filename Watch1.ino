#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <String.h>
#include <IPAddress.h>
#include <SimpleStack.h>
#include <cppQueue.h>
#include "config.h"
#include <ArduinoJson.h>

#define	IMPLEMENTATION	FIFO

// Replace with your network credentials
char* ssid = "pihotspot";
char* password = "c5k2mGwpG";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
TTGOClass *ttgo;
bool rtcIrq = false;
TFT_eSPI *tft;
BMA *sensor;
uint8_t prevRotation;
RTC_Date date;


String identity = "2";
String pc = "192.168.178.53";
String watch1 = "192.168.178.59";
String watch2 = "192.168.178.60";
String watch3 = "192.168.178.61";

String pi1 = "192.168.178.53";
String piPort = "80";


struct Event{
  String name;       //1. gyro 2. touch 3. joystick 4. accellerometer 5. humidty
  String origin;
  String data;
  int timestamp;
};

int queueSize = 1;
cppQueue eventQueue(sizeof(Event), 100, IMPLEMENTATION);	// Instantiate queue

int compEventRecieved = 0;

// event flags
String desiredGyro = "3";  // which gyro event is relevant, -1 for all
int timeframe = 10;   // how much time can pass between two relevant events
int op = 2;           // 1: AND  2: SEQ
int seq = 1;          // which event should come first in a seq   1: gyro first  2:  touch first

int getTimeInSeconds(){
  int sec = date.second + (date.minute * 60);
  return sec;
}

void sendCompEvent(Event touch, Event gyro){
  String payload = "Event=comp&Touch=True&Gyro=" + String(desiredGyro) + "&Timestamp=" + String(getTimeInSeconds()) + "&Origin=" + identity;
  //Serial.println(payload);
  sendEvent(payload, "comp");
}

void sendPiEvent(Event touch, Event gyro, Event timer){
  StaticJsonDocument<2048> jsonDoc;
  JsonObject touchJson = jsonDoc.createNestedObject("Touch");
  if(touch.data == ""){
    touchJson["Value"] = "True";
  }else{
    touchJson["Value"] = touch.data;

  }

  if(touch.timestamp == 0){
    touchJson["Timestamp"] = 420;
  }else{
    touchJson["Timestamp"] = touch.timestamp;

  }

  if(touch.origin == ""){
    touchJson["Origin"] = identity;
  }else{
    touchJson["Origin"] = touch.origin;

  }

  JsonObject gyroJson = jsonDoc.createNestedObject("Gyro");

  if(gyro.data == ""){
    gyroJson["Value"] = desiredGyro;
  }else{
    gyroJson["Value"] = gyro.data;

  }

  if(gyro.timestamp == 0){
    gyroJson["Timestamp"] = 420;
  }else{
    gyroJson["Timestamp"] = gyro.timestamp;

  }

  if(gyro.origin == ""){
    gyroJson["Origin"] = identity;
  }else{
    gyroJson["Origin"] = gyro.origin;
  }

  JsonObject timerJson = jsonDoc.createNestedObject("Timer");
  if(timer.data == ""){
    timerJson["Value"] = "True";
  }else{
    timerJson["Value"] = timer.data;
  }

  if(timer.timestamp == 0){
    timerJson["Timestamp"] = 420;
  }else{
    timerJson["Timestamp"] = timer.timestamp;
  }

  if(timer.origin == ""){
    timerJson["Origin"] = identity;
  }else{
    timerJson["Origin"] = timer.origin;
  }

  timerJson["Value"] = timer.data;
  timerJson["Timestamp"] = timer.timestamp;
  timerJson["Origin"] = timer.origin;

  jsonDoc["Timestamp"] = getTimeInSeconds();

  sendPi(jsonDoc, pi1, piPort);
}

void addEventAndCheck(Event e){
  eventQueue.push(&e);
  queueSize++;

  Event touch;
  Event gyro;
  Event timer;

  Event ev;

  int dummyCounter = 0;
  while(dummyCounter < 2){
    eventQueue.pop(&ev);
    queueSize--;
    if(ev.name == "dummy"){
      eventQueue.push(&ev);
      queueSize++;
      dummyCounter++;
      continue;
    }

    //while the not the dummy event, check if the timestamp is too old
    if((getTimeInSeconds() - ev.timestamp) < timeframe){

      //check for sequences of events
      if(op == 2){
        if(seq == 1){
          if((gyro.data == desiredGyro) && ev.name == "Touch"){
            
            sendPiEvent(touch, gyro, timer);
            
          }
        }else if(seq == 2){

          if(touch.data == "True" && (ev.data == desiredGyro)){
            sendPiEvent(touch, gyro, timer);
          }

        }else{
          Serial.println("Unknown sequence");
        }
      }

      //set flags for events
      if(ev.name == "Touch"){
        touch = ev;
      }else if (ev.name == "gyro"){
        gyro = ev;
      }else if(ev.name == "timer"){
        timer = ev;
      }else{
        Serial.println("Unknown event");
        Serial.println(ev.name);
      }
      
      //send if both events are present in a given timeframe (timeframe here being the max timestamp)
      if(op == 1){
        if(touch.data == "True" && (gyro.data == desiredGyro)){
            sendPiEvent(touch, gyro, timer);
        }
      }

      Serial.println("Timer.Data: " + timer.data);
      Serial.println("compEventRecieved: " + compEventRecieved);
      if(compEventRecieved > 0 && timer.data == "true"){
        compEventRecieved--;
        sendPiEvent(touch, gyro, timer);
        Serial.println(compEventRecieved);

      }
      eventQueue.push(&ev);
      queueSize++;
    }
  }
  Serial.println(queueSize);
}

char* stringToCharPointer(const String& str) {
  // Use the c_str() function to get a pointer to the character array
  return const_cast<char*>(str.c_str());
}


int sendPi(StaticJsonDocument<2048> jsonDoc, String ip, String port){
  if (WiFi.status() == WL_CONNECTED) {
      //TODO jsonDoc anpassen auf das echte payload

      String jsonStr;
      serializeJson(jsonDoc, jsonStr);

      String address = "http://";
      address += ip;
      address += ":";
      address += port;

      Serial.println(address);
      Serial.println(jsonStr);

      HTTPClient http;
      http.begin(address);  // Replace with your server URL
      http.addHeader("Content-Type", "application/json");    

      int httpResponseCode = http.POST(jsonStr);
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.print("Error: ");
        Serial.println(httpResponseCode);
      }
  }else{
    return -1;
  }
  return 0;
}


int sendHttp(String payload, String ip, String port){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String dest = "http://";
    dest = dest + ip + ":" + port;
    Serial.println(payload);
    Serial.println(dest);
    http.begin(dest);  // Replace with your server URL
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }else{
    return -1;
  }
  return 0;
}

void sendEvent(String payload, String type){
  Serial.println(payload);
  if(identity == "1"){

    if(type == "comp"){
      //event is complex and has to be sent to super 
      sendHttp(payload, watch2, "80/event");
    }else{
      //simple event to partner
      sendHttp(payload, watch3, "80/event");
    }

  }else if(identity == "2"){

    sendHttp(payload, watch1, "80/event");
    sendHttp(payload, watch3, "80/event");

  }else if(identity == "3"){
    if(type == "comp"){
      sendHttp(payload, watch2, "80/event");
    }else{
      sendHttp(payload, watch1, "80/event");
    }
  }
}

void watchSetup(){
  Serial.begin(115200);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->rtc->setDateTime(0, 0, 0, 0, 0, 0);

  // Gyro parameters
  tft = ttgo->tft;
  sensor = ttgo->bma;
  Acfg cfg;
  cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  cfg.range = BMA4_ACCEL_RANGE_2G;
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;
  sensor->accelConfig(cfg);
  sensor->enableAccel();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
}

void setup(){
  // Serial port for debugging purposes
  watchSetup();

  //push dummy event on queue
  Event d;
  d.origin = "dummy";
  d.timestamp = 0;
  d.name  = "dummy";
  d.data = "dummy";

  eventQueue.push(&d);


  //set time
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

  //set ip for a device
  server.on("/ip", HTTP_POST, [](AsyncWebServerRequest *request){
    String ip = request->getParam("ip", true)->value();
    String device = request->getParam("device", true)->value();
    Serial.println(device + " found on IP: " + ip);
    
    if(device.equals("watch1")){
      watch1 = ip;
      Serial.print("Watch 1: ");
      Serial.println(watch1);

    }else if(device.equals("watch2")){
      watch2 = ip;
      Serial.print("Watch 2: ");
      Serial.println(watch2);

    }else if(device.equals("pi1")){
      pi1 = ip;
      Serial.print("Pi1: ");
      Serial.println(pi1);

    }else if(device.equals("watch3")){
      watch3 = ip;
      Serial.print("Watch3: ");
      Serial.println(watch3);
    }

    request->send_P(200, "text/html", "ip set");
  });


    server.on("/params", HTTP_POST, [](AsyncWebServerRequest *request){
    String type = request->getParam("type", true)->value();
    String payload = request->getParam("payload", true)->value();

    if(type == "gyro"){
      desiredGyro = payload;
    }else if(type == "timeframe"){
      timeframe = payload.toInt();
    }else if(type == "port"){
      piPort = payload.toInt();
    }

    request->send_P(200, "text/html", "ip set");
  });


  server.on("/event", HTTP_POST, [](AsyncWebServerRequest *request){
    AsyncWebParameter* param = request->getParam("Event", true);
    String eventParam = param->value();
    Event e;
    e.name = eventParam;
    if(eventParam == "Touch"){
      AsyncWebParameter* value = request->getParam("Value", true);
      AsyncWebParameter* timestamp = request->getParam("Timestamp", true);
      AsyncWebParameter* origin = request->getParam("Origin", true);

      Serial.println(eventParam);
      Serial.println(value->value());
      Serial.println(timestamp->value());
      Serial.println(origin->value());

      e.data = value->value();
      e.timestamp = timestamp->value().toInt();
      e.origin = origin->value();

      addEventAndCheck(e);

    }else if(eventParam == "comp"){
      
      AsyncWebParameter* touch = request->getParam("Touch", true);
      AsyncWebParameter* gyro = request->getParam("Gyro", true);
      AsyncWebParameter* timestamp = request->getParam("Timestamp", true);
      AsyncWebParameter* origin = request->getParam("Origin", true);

      compEventRecieved++;
      Serial.println(compEventRecieved);

      Serial.println(eventParam);
      Serial.println(touch->value());
      Serial.println(gyro->value());
      Serial.println(timestamp->value());
      Serial.println(origin->value());

    }else{
      Serial.println("Unknown Event");
      Serial.println(eventParam);
    }

    request->send_P(200, "text/html", "event recieved");
  });



  //get Identity
  server.on("/getIdentity", HTTP_POST, [](AsyncWebServerRequest *request){
    if(identity == "1"){
      request->send_P(200, "text/html", "1");
    }else if(identity == "2"){
      request->send_P(200, "text/html", "2");
    }else{
      request->send_P(200, "text/html", "3");
    }
  });
  
  //switch networks
  server.on("/network", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", "Network changing");
    Serial.println("Test");
    char* name = "Galaxy A71EDC3";
    char* pw  = "eumj6682"; 

    ssid = name;
    password = pw;

    Serial.println(ssid);
    Serial.println(password);
    WiFi.disconnect();
  });

  // Start server
  server.begin();
  Serial.println(identity);

}

// ------------------------------------------------------------------------------------------------------------------------------------------

int loopCounter = 0;

void loop() {
  Serial.println(".");

  //Verbinden mit dem aktuellen Netwerk
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println(ssid);
      Serial.println(password);
    }
    Serial.println("Connected");
  }

  //clock signal only on in clock 1
  date = ttgo->rtc->getDateTime();
  if(date.second%10 == 0){
    if(identity == "2" && (loopCounter%4) == 0){
      compEventRecieved = 0;
      Event timer = {"timer", identity, "true", getTimeInSeconds()};
      addEventAndCheck(timer);
    }
  }

  //send touch event
  if(ttgo->touched()){
    Serial.println("Touch");
    Event x;
    x.name = "Touch";
    x.origin = identity;
    x.data = "true";
    x.timestamp = getTimeInSeconds();

    String payload = "Event=Touch&Value=True&Timestamp=" + String(getTimeInSeconds()) + "&Origin=" + identity;
    sendEvent(payload, "Touch");

    addEventAndCheck(x);
  }
  
  //rotation sensor
  if (prevRotation != sensor->direction()) {
    prevRotation = sensor->direction();
    Event x;
    x.origin = identity;
    switch (sensor->direction()) {
      case DIRECTION_DISP_DOWN:
          Serial.println("Display down");
          x.data = "1";
          break;
      case DIRECTION_BOTTOM_EDGE:
          Serial.println("Bottom Edge down");
          tft->setRotation(WATCH_SCREEN_BOTTOM_EDGE);
          x.data = "2";
          break;
      case DIRECTION_TOP_EDGE:
          Serial.println("Top Edge down");
          tft->setRotation(WATCH_SCREEN_TOP_EDGE);
          x.data = "3";
          break;
      case DIRECTION_RIGHT_EDGE:
          Serial.println("Right Edge down");
          tft->setRotation(WATCH_SCREEN_RIGHT_EDGE);
          x.data = "4";
          break;
      case DIRECTION_LEFT_EDGE:
          Serial.println("Left Edge down");
          tft->setRotation(WATCH_SCREEN_LEFT_EDGE);
          x.data = "5";
          break;
      default:
          break;
    }

    if(x.data != NULL){
      x.name = "gyro";
      x.timestamp = getTimeInSeconds();
      addEventAndCheck(x);
    }
  }
  loopCounter++;
  delay(250);
}
