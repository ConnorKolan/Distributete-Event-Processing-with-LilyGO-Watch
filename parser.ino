#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <String.h>
#include <IPAddress.h>
#include <SimpleStack.h>

#include "config.h"


// Replace with your network credentials
const char* ssid = "FRITZ!Box 7590 NK";
const char* password = "AssB!bERl!n($)";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

TTGOClass *ttgo;
bool rtcIrq = false;

TFT_eSPI *tft;
BMA *sensor;
uint8_t prevRotation;

RTC_Date date;


String identity = "1";
String pc = "192.168.178.53";
String watch1 = "192.168.178.59";
String watch2 = "192.168.178.58";
String watch3 = "192.168.178.60";
String pi1 = "192.168.178.40";
String pi2 = "192.168.178.41";

struct event{
  String name;       //1. gyro 2. touch 3. joystick 4. accellerometer 5. humidty
  String origin;
  String data;
  int timestamp;
};

SimpleStack<event> events(100);

void addEvent(event e){
  events.push(e);
  //Serial.println("Event added");
}

void handleEvents(){
  event e;
  int eventOne = 0;
  int eventTwo = 0;
  while(events.pop(&e)){
    //Watch 1
    Serial.print(e.name);
    Serial.print(": ");
    Serial.println(e.data);
    if(e.name == "Gyro" && e.data == "Left"){
      eventOne = 1;
    }

    if(e.name == "Gyro" && e.data == "Top"){
      eventTwo = 1;
    }

    if(eventOne && eventTwo){
      eventOne = 0;
      eventTwo = 0;
      Serial.println("Event fulfilled");
      sendHttp("Event fulfilled", "192.168.178.53", "5555");
    }
  }
}

int getTimeInSeconds(){
  return date.second;
}

int sendHttp(String payload, String ip, String port){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String dest = "http://";
    dest = dest + ip + ":" + port;

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

void sendEvent(String payload){
  payload = payload + "&Timestamp=" + String(getTimeInSeconds()) + "&Origin=" + identity;
  Serial.println(payload);
  if(identity == "1"){
    //sendHttp(payload, pc, "5555");

    //sendHttp(payload, watch2, "80/event");
    sendHttp(payload, watch3, "80/event");

  }else if(identity == "2"){
    //sendHttp(payload, pc, "5555");

    //sendHttp(payload, watch1, "80/event");
    //sendHttp(payload, watch3, "80/event");

  }else if(identity == "3"){
    sendHttp(payload, pc, "5555");

    //sendHttp(payload, watch1, "80/event");
    //sendHttp(payload, watch2, "80/event");
  }
}

void setup(){
  // Serial port for debugging purposes
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

    }else if(device.equals("pi2")){
      pi2 = ip;
      Serial.print("Pi2: ");
      Serial.println(pi2);
    }else if(device.equals("watch3")){
      watch3 = ip;
      Serial.print("Watch3: ");
      Serial.println(watch3);
    }else if(device.equals("identity")){
      identity = ip;
      Serial.print("Identity: ");
      Serial.println(identity);
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
    /*Sensors:
            - Gyro
            - Touch
            - Time 
    */

    

    request->send_P(200, "text/html", "event recieved");
  });

  server.on("/siddhi", HTTP_POST, [](AsyncWebServerRequest *request){
  });
  // Start server
  server.begin();
  
}

void loop() {

  //clock signal only on in clock 1
  date = ttgo->rtc->getDateTime();
  if(date.second%10 == 0){
    handleEvents();
  }

  //touch sensor
  if(ttgo->touched()){
    Serial.println("Touch");
    event x;
    x.name = "Touch";
    x.data = "True";
    addEvent(x);

    sendEvent("Event=Touch&Value=True");
  }
  
  //rotation sensor
  if (prevRotation != sensor->direction()) {
    prevRotation = sensor->direction();
    event x;
    switch (sensor->direction()) {
      case DIRECTION_DISP_DOWN:
          //No use
          break;
      case DIRECTION_DISP_UP:
          //No use
          break;
      case DIRECTION_BOTTOM_EDGE:
          Serial.println("Bottom Edge down");
          tft->setRotation(WATCH_SCREEN_BOTTOM_EDGE);
          x.data = "Bot";
          break;
      case DIRECTION_TOP_EDGE:
          Serial.println("Top Edge down");
          tft->setRotation(WATCH_SCREEN_TOP_EDGE);
          x.data = "Top";

          break;
      case DIRECTION_RIGHT_EDGE:
          Serial.println("Right Edge down");
          tft->setRotation(WATCH_SCREEN_RIGHT_EDGE);
          x.data = "Right";
          break;
      case DIRECTION_LEFT_EDGE:
          Serial.println("Left Edge down");
          tft->setRotation(WATCH_SCREEN_LEFT_EDGE);
          x.data = "Left";
          break;
      default:
          break;
    }
    if(x.data != NULL){
      x.name = "Gyro";
      addEvent(x);
      String payload = "Event=Gyro&Value=" + x.data;
      sendEvent(payload);
    }
  }
  delay(1000);
}
