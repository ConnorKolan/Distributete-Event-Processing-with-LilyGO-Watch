#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>

#include "config.h"
#include <String.h>
#include <IPAddress.h>
#include <SimpleStack.h>
//plan:
//      - stack of events and stack of operations
//      - handle these like the calculator
//      - periodically perform the check
//      - if all criteria are met send to destination

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

//IPs of the other devices
IPAddress watch1(192, 168, 0, 1);
IPAddress watch2(192, 168, 0, 2);
IPAddress pi1(192, 168, 0, 4);
IPAddress pi2(192, 168, 0, 5);

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

int sendHttp(String payload, String ip, String port){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String dest = "http://";
    dest = dest + ip + port;

    http.begin(dest);  // Replace with your server URL
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String payload = "payload";  // Replace with your payload data
    
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
    /*Sensors:
            - Gyro
            - Touch
            - Time 
    */
    String event = request->getParam("event", true)->value();
    String value = request->getParam("value", true)->value();
    String timestamp = request->getParam("timestamp", true)->value();

    Serial.println("New " + event + " event with value " + value + "recieved at " + timestamp);
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
    Serial.println("ping");
    handleEvents();
  }

  //touch sensor
  if(ttgo->touched()){
    Serial.println("Touch");
    event x;
    x.name = "Touch";
    x.data = "True";
    addEvent(x);

    //sendHttp("Touched=True");
  }
  
  //rotation sensor
  if (prevRotation != sensor->direction()) {
    prevRotation = sensor->direction();
    event x;
    x.name = "Gyro";
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
    addEvent(x);
  }
  
  delay(1000);
}
