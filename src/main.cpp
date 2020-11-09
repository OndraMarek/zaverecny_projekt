// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <SPI.h>

#include <NTPClient.h>
#include "WiFiUdp.h"




// Replace with your network credentials
const char* ssid = "Tomas_WiFi";
const char* password = "7405145473";




byte inputHodiny=12;
byte inputMinuty=0;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");



// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String getTime() {
  String time = timeClient.getFormattedTime();
  Serial.println(time);
  return String(time);
}




// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  else if (var == "TIME"){
    return getTime();
  }
}
 
void setup(){

  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);


  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  timeClient.begin();
  timeClient.setTimeOffset(3600);

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTime().c_str());
  });

////TEST/////
  server.on("/hodiny", HTTP_POST, [](AsyncWebServerRequest *request) {
      inputHodiny = request->arg("hodiny").toInt();
      request->send_P(200, "text/json", "{\"result\":\"ok\"}");
    });

  server.on("/minuty", HTTP_POST, [](AsyncWebServerRequest *request) {
      inputMinuty = request->arg("minuty").toInt();
      request->send_P(200, "text/json", "{\"result\":\"ok\"}");
    });
////TEST/////

  // Start server
  server.begin();

}
 
void loop(){
  timeClient.update();
  Serial.println(inputHodiny);
  Serial.println(inputMinuty);
 
  delay(1000);
}