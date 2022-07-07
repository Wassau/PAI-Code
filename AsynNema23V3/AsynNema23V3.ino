//////////////////////////////
// Settings  motors///////
//////////////////////////

#include "Arduino.h"
#include <WiFi.h>       // standard library
#include <WebServer.h>  // standard library
#include "indexAJAX.h"   // .h file that stores your html page code
#include "funcionesMotores.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SolarCalculator.h>
#include <TimeLib.h>



///////////////////////////
//Setting Webserver, websocket.///
//////////////////////////////////

AsyncWebServer server(80);
// replace this with your homes intranet connect parameters
#define LOCAL_SSID "FamiliaArcila"
#define LOCAL_PASS "51710096"

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Heliodon"
#define AP_PASS "16051984"


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

// variables to store measure data and sensor states
int BitsPOT = 0, BitsA1 = 0;
float VoltsPOT = 0, VoltsA1 = 0;
int FanSpeed = 0;
bool LED0 = false, SomeOutput = false, isOnNORTH = false, isOnSOUTH = false, isOnWEST = false, isOnEAST = false;
uint32_t SensorUpdate = 0;
int FanRPM = 0;

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char buf[32];

// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

///////////////
// Variables ////
//////////////////


bool moveR = false;
bool moveL =  false;
bool moveforwards = false;
bool moveback = false;
// Encoder:
int psCarro = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(DIR_STPM, OUTPUT);
  pinMode(EN_STPM, OUTPUT);
  pinMode(PULSE, OUTPUT);
  pinMode(pinNorth, INPUT_PULLUP);
  pinMode(Carromas, INPUT_PULLUP);
  pinMode(pinSouth, INPUT_PULLUP);
  pinMode(Carromenos, INPUT_PULLUP);
  attachInterrupt(EMERGENCY_BUTTON_PIN, ISR_EMERGENCY, FALLING);
  attachInterrupt(pinNorth, ISR_ProcessButton_NORTH , RISING);
  attachInterrupt(Carromas, ISR_ProcessButton_WEST, RISING);
  attachInterrupt(pinSouth, ISR_ProcessButton_SOUTH, RISING);
  attachInterrupt(Carromenos, ISR_ProcessButton_EAST, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(Limcarro), Limitcarro, RISING);
  attachInterrupt(digitalPinToInterrupt(Limcarro), Limitarco, RISING);
  setupMotors();
  digitalWrite(DIR_STPM, HIGH);
  pinMode(EN_STPM,LOW);

  disableCore1WDT();

  // just an update to progress
  Serial.println("starting server");

  // if you have this #define USE_INTRANET,  you will connect to your home intranet, again makes debugging easier
#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Actual_IP = WiFi.localIP();
#endif

  // if you don't have #define USE_INTRANET, here's where you will creat and access point
  // an intranet with no internet connection. But Clients can connect to your intranet and see
  // the web page you are about to serve up
#ifndef USE_INTRANET
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: "); Serial.println(Actual_IP);
#endif


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", PAGE_MAIN );
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
        ISR_ProcessButton_NORTH();
        break;
      case 33:
        ISR_ProcessButton_SOUTH();
        break;
      case 32:
        ISR_ProcessButton_EAST();
        break;
      case 26:
        ISR_ProcessButton_WEST();
        break;
    }
  });
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


    request->send(200, "text/plain", "OK");
  });


  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent

  // finally begin the server
  server.begin();





}

void loop() {

  //    if (!IS_EMERGENCY) {
  //    if (counter == -1) {
  //      if (getIsManual()) {
  //        Serial.println(F("is manual"));
  //        manualMov();
  //      } else {
  //       // automaticMov(42.05, 297.38);
  //      }
  //    }
  //
  //  }
  //  counter++;
  //}
  manualMov();
  digitalWrite(DIR_STPM, HIGH);
  digitalWrite (EN_STPM, HIGH);
  ledcWrite(MPPChannel, 100);
  delay(50);
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


  ///////////////////////////////
  // MOVIMIENTO MOTOR DE ARCO
  //////////////////////////////
  int numStepsArcAux = numStepsArc() * 20;
  Serial.print(F("numStepsArcAux: "));
  Serial.println(numStepsArcAux);

  Serial.print(F("motorArcMov: "));
  if (numStepsArcAux > 0) {
    for (int i = 0; i < numStepsArcAux; i++) {
      digitalWrite(PULSE, LOW);
      digitalWrite(PULSE, HIGH);
      delayMicroseconds(60);
      Serial.println(i);

    }
  } else {
    for (int i = 0; i > numStepsArcAux; i--) {
      //myStepper.step(-1);
      Serial.println(i);

    }
  }

  ///////////////////////////////
  // MOVIMIENTO MOTOR DC
  //////////////////////////////

  //int numPulsesCarAux = numPulsesCar();
  int numPulsesCarAux = -20;
  Serial.print(F("numPulsesCarAux: "));
  Serial.println(numPulsesCarAux);

  Serial.print(F("DC motor: "));
  if (numPulsesCarAux > 0) {
    setMovDCMotorForward();
    for (int i = 0; i < numPulsesCarAux; i++) {
      ledcWrite(dcMotorChannel, 255);
      Serial.println(i);

    }
    stopDCMotor();
  } else {
    setMovDCMotorBackwards();
    for (int i = 0; i > numPulsesCarAux; i--) {
      ledcWrite(dcMotorChannel, 255);
      Serial.println(i);

    }
    stopDCMotor();
  }

}

//////////////////
// Sensors /
//////////////////
void readEncoder() {
  int b = digitalRead(ENCB); // Calcula la posición en Grados del motor (verificar PPR)
  if (b > 0) {
    posCarro++;
  }
  else {
    posCarro--;
  }
}

void manualMov() {

  if (moveR) {
//    digitalWrite(DIR_STPM, HIGH);
//    digitalWrite (EN_STPM, HIGH);
//    ledcWrite(MPPChannel, 100);
    pinMode(DIR_STPM, HIGH);
    digitalWrite(PULSE, LOW);
    digitalWrite(PULSE, HIGH);
    delayMicroseconds(60);

    moveR = false;
  }
  if (moveL) {
//    digitalWrite(DIR_STPM, LOW);
//    digitalWrite (EN_STPM, HIGH);
//    //    for (int x = 0; x < 100 ; x++) { // define CCW or CW rotation
//    //      digitalWrite(PULSE, HIGH);
//    //      digitalWrite (EN_STPM, HIGH);
//    //      delayMicroseconds(100);
//    //      digitalWrite(PULSE, LOW);
//    //      digitalWrite (EN_STPM, LOW);
//    //      delayMicroseconds(100);
//    //    }
//    ledcWrite(MPPChannel, 100);
//    delay(20);

    pinMode(DIR_STPM, LOW);
    digitalWrite(PULSE, LOW);
    digitalWrite(PULSE, HIGH);
    delayMicroseconds(60);
    moveL = false;
  }
  if (moveforwards) {

    digitalWrite(pinDCMotorDriveIN1, LOW ); //Switch between this HIGH and LOW to change direction
    digitalWrite(pinDCMotorDriveIN2,  HIGH);
    ledcWrite(dcMotorChannel, 180);
    delay(100);
    digitalWrite(pinDCMotorDriveIN1, LOW );
    digitalWrite(pinDCMotorDriveIN2,  LOW);

    moveforwards = false;
  }
  if (moveback) {
    digitalWrite(pinDCMotorDriveIN1, HIGH); //Switch between this HIGH and LOW to change direction
    digitalWrite(pinDCMotorDriveIN2, LOW);
    ledcWrite(dcMotorChannel, 180);
    delay(100);
    digitalWrite(pinDCMotorDriveIN1, LOW); //STOP
    digitalWrite(pinDCMotorDriveIN2, LOW);
    moveback = false;
  }
}

void ISR_ProcessButton_EAST() {

  moveforwards = true;
}
void ISR_ProcessButton_WEST() {

  moveback = true;
}
void ISR_ProcessButton_SOUTH() {
  moveL = true;
}
void ISR_ProcessButton_NORTH() {
  moveR = true;
}

int correctedHour(int utcHour) {
  return utcHour - utc_offset;
  //return utcHour;
}


//
