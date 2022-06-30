/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "indexAJAX.h"

#include <SolarCalculator.h>
#include <TimeLib.h>

#include "funcionesMotores.h"

#define AP_SSID "Heliodon"
#define AP_PASS "16051984"

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_INPUT_3 = "value"; //opcional con el id se puede

const char* PARAM_LAT = "lat";
const char* PARAM_LNG = "lng";
const char* PARAM_DAY = "current_day";
const char* PARAM_TIME = "current_time";

String PARAM_LAT_VAL;
String PARAM_LNG_VAL;
String PARAM_DAY_VAL;
String PARAM_TIME_VAL;

double LAT;
double LNG;
int current_time[3];
int current_date[3];

int utc_offset;
time_t utc;
double az, el;


// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  setupMotors();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: "); Serial.println(Actual_IP);


  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", PAGE_MAIN);
  });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage1;
    String inputMessage2;
    String inputMessage3;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    switch (inputMessage1.toInt()) {
      case 25:
        ProcessButton_NORTH();
        break;
      case 33:
        ProcessButton_SOUTH();
        break;
      case 32:
        ProcessButton_EAST();
        break;
      case 26:
        ProcessButton_WEST();
        break;
    }

    Serial.print("ID: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {

    String inputMessage;
    String inputParam;

    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_LAT) && request->hasParam(PARAM_LNG) &&
        request->hasParam(PARAM_DAY) && request->hasParam(PARAM_TIME)) {
      PARAM_LAT_VAL = request->getParam(PARAM_LAT)->value();
      PARAM_LNG_VAL = request->getParam(PARAM_LNG)->value();
      PARAM_DAY_VAL = request->getParam(PARAM_DAY)->value();
      PARAM_TIME_VAL = request->getParam(PARAM_TIME)->value();
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.print(F("PARAM_LAT_VAL: "));
    Serial.println(PARAM_LAT_VAL);
    Serial.print(F("PARAM_LNG_VAL: "));
    Serial.println(PARAM_LNG_VAL);
    Serial.print(F("PARAM_DAY_VAL: "));
    Serial.println(PARAM_DAY_VAL);
    Serial.print(F("PARAM_TIME_VAL: "));
    Serial.println(PARAM_TIME_VAL);
    Serial.println(F("------------------"));
    convertions();

    request->send(200, "text/html", PAGE_MAIN);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {

}

void ProcessButton_NORTH() {
  myStepper.step(1);
  Serial.println("estas en elProcess_North");
}


void ProcessButton_SOUTH() {
  myStepper.step(-1);
  Serial.println("estas en elProcess_South");
}

void ProcessButton_WEST() {
  for (int i = 0; i <= 10; i++) {
    pwmMovForward(1);
  }
  Serial.println("estas en elProcess_West");
}

void ProcessButton_EAST() {
  for (int i = 0; i <= 10; i++) {
    pwmMovBackwards(1);
  }
  Serial.println("estas en elProcess_East");
}


void convertions() {
  LAT = PARAM_LAT_VAL.toDouble();
  LNG = PARAM_LNG_VAL.toDouble();
  const char* day_char = PARAM_DAY_VAL.c_str();
  const char* time_char = PARAM_TIME_VAL.c_str();
  sscanf(day_char, "%d-%d-%d", &current_date[0], &current_date[1], &current_date[2]);//año-mes-dia
  sscanf(time_char, "%d:%d:%d", &current_time[0], &current_time[1], &current_time[2]);//horas-minutos-segundos

  utc_offset = (int)floor(LNG / 15);
  setTime(correctedHour(current_time[0]), current_time[1], current_time[2], current_date[2], current_date[1], current_date[0]);
  utc = now();
  calcHorizontalCoordinates(utc, LAT, LNG, az, el);

  elNext = el;
  azNext = az;
  Serial.print(F("elNext: "));
  Serial.println(elNext);
  Serial.print(F("azNext: "));
  Serial.println(azNext);
  int numStepsArcAux = numStepsArc()*20;
  Serial.print(F("numStepsArcAux: "));
  Serial.println(numStepsArcAux);

  Serial.print(F("motorArcMov: "));
  if (numStepsArcAux > 0) {
    for (int i = 0; i < numStepsArcAux; i++) {
      myStepper.step(1);
      Serial.println(i);
      delay(15);
    }
  } else {
    for (int i = 0; i > numStepsArcAux; i--) {
      myStepper.step(-1);
      Serial.println(i);
      delay(15);
    }
  }
  //int numPulsesCarAux = numPulsesCar();
  int numPulsesCarAux = -20;
  Serial.print(F("numPulsesCarAux: "));
  Serial.println(numPulsesCarAux);

  Serial.print(F("DC motor: "));
  if (numPulsesCarAux > 0) {
    setMovDCMotorForward();
    for (int i = 0; i < numPulsesCarAux; i++) {
      ledcWrite(dcMotorChannel, 200);
      Serial.println(i);
      delay(15);
    }
    stopDCMotor();
  } else {
    setMovDCMotorBackwards();
    for (int i = 0; i > numPulsesCarAux; i--) {
      ledcWrite(dcMotorChannel, 255);
      Serial.println(i);
      delay(15);
    }
    stopDCMotor();
  }

}

int correctedHour(int utcHour) {
  return utcHour - utc_offset;
  //return utcHour;
}
