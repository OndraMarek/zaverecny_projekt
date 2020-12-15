#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
/*#include <FS.h>
#include <Wire.h>
#include <SPI.h>*/

#include <NTPClient.h>
#include "WiFiUdp.h"
#include <Servo.h>

#include <ESPAsyncWiFiManager.h>

/*const char* ssid = "SSPUOpava";
const char* password = "";*/

byte inputHodiny;
byte inputMinuty;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

//TEST
const uint8_t servoPin = D2;
Servo servo;

int otevrit=180;
int zavrit=0;
//TEST

// Set LED GPIO
const int ledPin = 2;
// Stores LED state
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
DNSServer dns;

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
    servo.write(otevrit);
    delay(1000);
    servo.write(zavrit);
  }
}

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
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void wificonnect(){
  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");
  Serial.println(WiFi.localIP());
}

void htmlRequests(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/main.js", String());
  });

//TEST//////////// 
  /*server.on("/angle", HTTP_POST, [](AsyncWebServerRequest *request) {
    String angle = request->arg("angle");
    Serial.println("Current Position: " + angle + "°");
    servo.write(angle.toInt());
    request->send(200);
  });*/
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
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  servo.attach(servoPin);

  initSpiffs();

  wificonnect();


  timeClient.begin();
  timeClient.setTimeOffset(3600);


  htmlRequests();

  server.begin();
}
 
void loop(){
  timeClient.update();
  Serial.println(inputHodiny);
  Serial.println(inputMinuty);
  feed();


  delay(1000);
}