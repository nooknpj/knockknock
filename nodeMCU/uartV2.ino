#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;
SoftwareSerial NodeSerial(D2, D3); // D2->RX | D3->TX

const char* ssid = "nnook";
const char* password = "12345679";

const String ip = "172.20.10.2:3000";
const String target = "http://" + ip;
int doorStatus;
int prev=0;
int now=0;
int timeDiff=0;

void setup() {

  //set up for uart
  pinMode(D2, INPUT);
  pinMode(D3, OUTPUT);
  NodeSerial.begin(115200); // for uart comm

  Serial.begin(115200);

  //connect to WIFI
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

}

void loop() {
  // wait for WiFi connection

  //Serial.println("gogo");
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    //Get door status from server-------------------------------------------------------------------------------------------
    HTTPClient http;
    http.begin(target + "/status");
    int httpCode = http.GET();
    if (httpCode > 0) {

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        //Serial.println(payload);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
        int st = root["status"];
        doorStatus = st;
        //Serial.print("JSON doorStatus: ");
        //Serial.println(st);

      }
      //if request error
    } else Serial.printf("[ GET Status failed, error: %s\n", http.errorToString(httpCode).c_str());

    http.end();



    //Get Status from server and transmit doorStatus to STM32 ---------------------------------------------------------------

    //if doorStatus = 1 (Open)
    String c = "x";
    if (doorStatus) {
      Serial.println("Open the door");
      Serial.println(doorStatus); // transmit 1 to STM32
      c[0] = 0x01;
      NodeSerial.println(c);


    } else {
      //if doorStatus=0 (Close)
      Serial.println("Close the door");
      Serial.println(doorStatus); // transmit 0 to STM32
      c[0] = 0x02;
      NodeSerial.println(c);

    }

    
    // if receive signal from STM32 via Serial Communication (Detection and Get Door Status) ---------------------------
    // need to change Serial->NodeSerial
    if (NodeSerial.available() > 0) {
     
      char s[1];
      NodeSerial.readBytes(s, 1);
      Serial.println(s[0]);
      now = millis();
      
      timeDiff = now - prev;
      
      Serial.print("TimeDiff =");
      Serial.println(timeDiff);
      

      /*if (Serial.read() == '\n') {
        Serial.println(s[0]);
      }*/

      //case 0 : Detection
      if (s[0] == 0 && timeDiff >= 100) {
        Serial.println("CASE0: DETECTED");
        prev = now;

        HTTPClient http;
        http.begin(target + "/detected");
        int httpCode = http.GET();
        if (httpCode > 0) {
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            //Serial.print("Server response: ");
            //Serial.println(payload);
          }
          //if request error
        } //else Serial.printf("[HTTP] Detected, error: %s\n", http.errorToString(httpCode).c_str());
        http.end();

      }

      //case1 STM32:DoorStatus : Open
      if (s[0] == 1) {
        Serial.println("CASE1: OPEN");
        doorStatus = 1;

        HTTPClient http;
        http.begin(target + "/open");
        int httpCode = http.GET();
        if (httpCode > 0) {
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.print("Server response: ");
            Serial.println(payload);
          }

          //if request error
        } else Serial.printf("[HTTP] Open, error: %s\n", http.errorToString(httpCode).c_str());
        http.end();
      }


      //case2 STM32:DoorStatus : Close
      if (s[0] == 2) {
        Serial.println("CASE2: CLOSE");
        doorStatus = 0;

        HTTPClient http;
        http.begin(target + "/close");
        int httpCode = http.GET();
        if (httpCode > 0) {
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.print("Server response: ");
            Serial.println(payload);
          }
          //if request error
        } else Serial.printf("[HTTP] Close failed, error: %s\n", http.errorToString(httpCode).c_str());
        http.end();
      }

      
    }

    //-----------------------------------------------------------------------------------------------

  } else {
    Serial.println("Attempting to connect to Wifi");
  }

  //delay
  delay(500);


}
