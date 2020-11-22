// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <SPI.h>

#include <NTPClient.h>
#include "WiFiUdp.h"
#include <Servo.h>



// Replace with your network credentials
const char* ssid = "Tomas_WiFi";
const char* password = "7405145473";




byte inputHodiny=12;
byte inputMinuty=0;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");


//TEST
const uint8_t servoPin = D2;
/* Create Servo Object */
Servo servo;
//TEST

// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String getTime() {
  String time = timeClient.getFormattedTime();
  //int hh = timeClient.getHours();
  //int mm = timeClient.getMinutes();
  //String time = hh + ":" + mm;
  Serial.println(time);
  return String(time);
}

void feed(){
  if(inputHodiny== timeClient.getHours() && inputMinuty== timeClient.getMinutes()){
    Serial.println("Feeding");
  }
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

void initSpiffs(){
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void wificonnect(){
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
}

void htmlRequests(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

//TEST////////////
  server.on("/angle", HTTP_POST, [](AsyncWebServerRequest *request) {
    String angle = request->arg("angle");
    Serial.println("Current Position: " + angle + "°");
    servo.write(angle.toInt());
    request->send(200);
  });
//TEST/////////////

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTime().c_str());
  });

  server.on("/hodiny", HTTP_POST, [](AsyncWebServerRequest *request) {
      inputHodiny = request->arg("hodiny").toInt();
      request->send_P(200, "text/json", "{\"result\":\"ok\"}");
    });

  server.on("/minuty", HTTP_POST, [](AsyncWebServerRequest *request) {
      inputMinuty = request->arg("minuty").toInt();
      request->send_P(200, "text/json", "{\"result\":\"ok\"}");
    });
}
 
void setup(){

  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  servo.attach(servoPin);

  initSpiffs();

  wificonnect();

  timeClient.begin();
  timeClient.setTimeOffset(3600);

  Serial.println(WiFi.localIP());

  htmlRequests();


  // Start server
  server.begin();
}
 
void loop(){
  timeClient.update();
  Serial.println(inputHodiny);
  Serial.println(inputMinuty);
  feed();

  delay(60000);
}