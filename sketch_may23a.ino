#include <WiFi.h>
#include <SPI.h>
#include "config.h"

WiFiClient client;

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin("FRITZ!Box 7590 NK", "AssB!bERl!n($)");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Wi-Fi connected
  Serial.println("Connected to WiFi!");

  // Print the ESP32's IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  IPAddress server(192, 168, 178, 43);
  while(!client.connected()){
    Serial.println("Attempting to connect");
    client.connect(server, 6666);
    delay(2000);
  }

  Serial.println("connected");
  // Make a HTTP request:
    client.println("IBM;700;100");
    Serial.println("IBM;700;100");
    delay(2000);

    client.println("WSO2;60;200");
    Serial.println("WSO2;60;200");
    delay(2000);

    client.println("GOOG;50;30");
    Serial.println("GOOG;50;30");
    delay(2000);

    client.println("IBM;77;400");
    Serial.println("IBM;77;400");
    delay(2000);    
    
    client.println("WSO2;45;50");
    Serial.println("WSO2;45;50");
    delay(2000);
    
}

void loop() {

}
