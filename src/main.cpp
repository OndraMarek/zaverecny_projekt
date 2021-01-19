#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include "WiFiUdp.h"
#include <Servo.h>
#include <ESPAsyncWiFiManager.h>
#define DEBUG

byte inputHodiny;
byte inputMinuty;
String inputCas;
int pom = 0;
unsigned long previousMillis = 0;
unsigned long interval = 1000;
int i=0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org");

const uint8_t servoPin = D2;
Servo servo;

AsyncWebServer server(80);
DNSServer dns;

//Funkce pro spuštění krmení, vyvoláve se ve funkci feed()
void davka(){
  for(;i<8;){
      unsigned long currentMillis = millis();
      if(currentMillis - previousMillis > interval) {
          previousMillis = currentMillis;
          if (servo.read() == 180){
              servo.write(0);
              i++;
          }
          else{
              servo.write(180);
              i++;
          }
      }
      yield();
  }
}

//Ukládání času získaného z NTP serveru do proměnné time
String getTime(){
  String getHodiny;
  String getMinuty;
  if(timeClient.getHours()>9){
    getHodiny = String(timeClient.getHours());
  }
  else{
    getHodiny = "0"+String(timeClient.getHours());
  }

  if(timeClient.getMinutes()>9){
    getMinuty = String(timeClient.getMinutes());
  }
  else{
    getMinuty = "0"+String(timeClient.getMinutes());
  }
  String time = getHodiny+":"+getMinuty;
  return String(time);
}

//Funkce porovná čas získaný z webové aplikace a čas z NTP serveru, v případě že se časy rovnají spouští funkci pro krmení
void feed(){
  if (getTime() == inputCas && pom == 0){
    Serial.println("Feeding");
    davka();
    pom = 1;
    i=0;
  }
  if (getTime() != inputCas){
    pom = 0;
  }
}

//Pokud funkce najde na html stránce symbol %TIME%, nahradí jej časem z funkce getTime()
String processor(const String &var){
  Serial.println(var);
  if (var == "TIME"){
    return getTime();
  }
}

//Inicializace SPIFFS
void initSpiffs(){
  if (!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

//Funkce pro automatické připojení k WiFi síti pomoci knihovny ESPAsyncWiFiManager.h
void wificonnect(){
  AsyncWiFiManager wifiManager(&server, &dns);
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");
  Serial.println(WiFi.localIP());
}

//Inicializace NTP clientu a nastavení správného časového pásma 
void timeClientFunc(){
  timeClient.begin();
  timeClient.setTimeOffset(3600);
}

//HTTP GET a POST requesty
void htmlRequests(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/main.js", String());
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", getTime().c_str());
  });

  server.on("/cas", HTTP_POST, [](AsyncWebServerRequest *request) {
    inputCas = request->arg("cas");
    request->send_P(200, "text/json", "{\"result\":\"ok\"}");
  });
}

void setup(){
  Serial.begin(115200); 
  servo.attach(servoPin);

  initSpiffs();

  wificonnect();

  timeClientFunc();

  htmlRequests();

  server.begin();
}

void loop(){
  timeClient.update();
  feed();
}