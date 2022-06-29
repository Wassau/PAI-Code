#include "Arduino.h"
#include <WiFi.h>       // standard library
#include <WebServer.h>  // standard library
#include "indexAJAX.h"   // .h file that stores your html page code
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include "funcionesMotores.h"
////////////////////////////
// CONFIGURACIONES DE RED///
////////////////////////////

AsyncWebServer server(80);
// replace this with your homes intranet connect parameters
#define LOCAL_SSID "FamiliaArcila"
#define LOCAL_PASS "51710096"

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Heliodon"
#define AP_PASS "16051984"


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

// gotta create a server
WebServer server(80);

////////////////////////////////////////
//CONFIGURACIONES PINES Y VARIABLES ////
///////////////////////////////////////




// pins stepper motor
#define DIR_STPM 16 //  Motor Driver Pins
#define PULSE 4
#define EN_STPM 17
#define pinPot 32 // Current state is opne loop

int STEPS_PER_REV = 400;  // min number of steps per Rev, max 25000
//int Angletostep = 0;    // turn Orientation angle to Step
// pins  button
#define EMERGENCY_BUTTON_PIN 2
#define pinNorth 35
#define Carromas 36
#define pinSouth 39
#define Carromenos 34


// pins DC motor
#define pinDCMotorDriveEn 21
#define pinDCMotorDriveIN1 22
#define pinDCMotorDriveIN2 13
#define ENCA 14
#define ENCB 27
// ESP32 constants
#define analogInputMax 4095
// setting PWM properties
const int freq = 5000;
const int dcMotorChannel = 0;
const int resolution = 8;
int max_duty_cycle = (int)(pow(2, resolution) - 1);
float wait = (1.0 / max_duty_cycle) * 1000.0;

// setting PWM_STPM properties  // usually doesn't work
const int freq2 = 5000;
const int MotorChannel2 = 1;
const int resolution2 = 14;
int max_duty_cycle2 = (int)(pow(2, resolution) - 1);
float wait2 = (1.0 / max_duty_cycle) * 1000.0;


// global flags of interruption.
bool moveR = false;
bool moveL =  false;
bool moveforwards = false;
bool moveback = false;
// potenciometer not implemented :(
int Duty = 0;


bool IS_EMERGENCY = false; // EMERGENCY
int posCarro = 0; // encoder
volatile int pinNorthRedState = HIGH;
volatile int pinWestYellowState = HIGH;
volatile int pinSouthBlueState = HIGH;
volatile int pinEastGreenState = HIGH;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(DIR_STPM, OUTPUT);
  pinMode(EN_STPM, OUTPUT);
  pinMode(PULSE, OUTPUT);
  pinMode(pinDCMotorDriveIN1, OUTPUT);
  pinMode(pinDCMotorDriveIN2, OUTPUT);
  pinMode(pinNorth, INPUT_PULLUP);
  pinMode(Carromas, INPUT_PULLUP);
  pinMode(pinSouth, INPUT_PULLUP);
  pinMode(Carromenos, INPUT_PULLUP);
  attachInterrupt(EMERGENCY_BUTTON_PIN, ISR_EMERGENCY, FALLING);
  attachInterrupt(pinNorth, ISR_btnNorthRed, RISING);
  attachInterrupt(Carromas, ISR_btnWestYellow, RISING);
  attachInterrupt(pinSouth, ISR_btnSouthBlue, RISING);
  attachInterrupt(Carromenos, ISR_btnEastGreen, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
  ledcSetup(dcMotorChannel, freq, resolution);
  ledcAttachPin(pinDCMotorDriveEn, dcMotorChannel);
  ledcSetup(MotorChannel2, freq2, resolution2);
  ledcAttachPin(PULSE, MotorChannel2);


  //////////////////////////
  //CONFIGURACIONES WEB PAGE/////
  //////////////////////////

  // maybe disable watch dog timer 1 if needed
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

  printWifiStatus();


  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent
  server.on("/", SendWebsite);

  // upon esp getting /XML string, ESP will build and send the XML, this is how we refresh
  // just parts of the web page
  server.on("/xml", SendXML);

  // upon ESP getting /UPDATE_SLIDER string, ESP will execute the UpdateSlider function
  // same notion for the following .on calls
  // add as many as you need to process incoming strings from your web page
  // as you can imagine you will need to code some javascript in your web page to send such strings
  // this process will be documented in the SuperMon.h web page code
  //  server.on("/UPDATE_SLIDER", UpdateSlider);
  //  server.on("/BUTTON_0", ProcessButton_0);
  //  server.on("/BUTTON_1", ProcessButton_1);
  server.on("/BUTTON_NORTH", ProcessButton_NORTH);
  server.on("/BUTTON_SOUTH", ProcessButton_SOUTH);
  server.on("/BUTTON_WEST", ProcessButton_WEST);
  server.on("/BUTTON_EAST", ProcessButton_EAST);

  // finally begin the server
  server.begin();
  }

void loop() {
  // put your main code here, to run repeatedly:
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
  // if

  manualMov();

    // no matter what you must call this handleClient repeatidly--otherwise the web page
  // will not get instructions to do something
  server.handleClient();
}
//////////////////
// Motor Movement /
//////////////////


void setStepmotor(int dirPin, int Duty, int target) {
  if (dirPin == 1) {
    // Set motor direction clockwise
    Angletostep = (STEPS_PER_REV / 360) * target;
    digitalWrite (EN_STPM, HIGH);
    digitalWrite (DIR_STPM, HIGH);
    for (int x = 0; x < Angletostep ; x++) { // define CCW or CW rotation
      digitalWrite(PULSE, LOW);
      digitalWrite (EN_STPM, HIGH);
      delayMicroseconds(Duty);
      digitalWrite(PULSE, HIGH);
      digitalWrite (EN_STPM, LOW);
      delayMicroseconds(Duty);
    }
  }
  else {
    // Set motor direction counterclockwise
    digitalWrite (DIR_STPM, LOW);
    for (int x = 0; x < Angletostep ; x++) { // define CCW or CW rotation
      digitalWrite(PULSE, HIGH);
      digitalWrite (EN_STPM, HIGH);
      delayMicroseconds(Duty);
      digitalWrite(PULSE, LOW);
      digitalWrite (EN_STPM, LOW);
      delayMicroseconds(Duty);

    }
  }
}
void stopDCMotor() {
  Serial.println(F(" DC Motor stopped "));
  digitalWrite(pinDCMotorDriveIN1, LOW);
  digitalWrite(pinDCMotorDriveIN2, LOW);
}
void btnsUpdateState() {

  pinNorthRedState = digitalRead(pinNorthRedState);
  delay(15);
  pinWestYellowState = digitalRead(pinWestYellowState);
  delay(15);
  pinSouthBlueState = digitalRead(pinSouthBlueState);
  delay(15);
  pinEastGreenState = digitalRead(pinEastGreenState);
  delay(15);

  Serial.print(F("pinNorthRedState: "));
  Serial.println(pinNorthRedState);
  Serial.print(F("pinWestYellowState: "));
  Serial.println(pinWestYellowState);
  Serial.print(F("pinSouthBlueState: "));
  Serial.println(pinSouthBlueState);
  Serial.print(F("pinEastGreenState: "));
  Serial.println(pinEastGreenState);
}
void manualMov() {
  if (moveR) {
    digitalWrite(DIR_STPM, HIGH);
    digitalWrite (EN_STPM, HIGH);
    for (int x = 0; x < 100 ; x++) { // define CCW or CW rotation
      digitalWrite(PULSE, HIGH);
      digitalWrite (EN_STPM, LOW);
      delayMicroseconds(2);
      digitalWrite(PULSE, LOW);
      digitalWrite (EN_STPM, HIGH);
      delayMicroseconds(2);
    }
    delay(100);
    moveR = false;
  }
  if (moveL) {
    digitalWrite(DIR_STPM, LOW);
    digitalWrite (EN_STPM, HIGH);
    for (int x = 0; x < 100 ; x++) { // define CCW or CW rotation
      digitalWrite(PULSE, HIGH);
      digitalWrite (EN_STPM, HIGH);
      delayMicroseconds(2);
      digitalWrite(PULSE, LOW);
      digitalWrite (EN_STPM, LOW);
      delayMicroseconds(2);
    }
    delay(100);
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
void potToAngleUpdate() {
  //btnsUpdateState();
  Serial.print(F("pinNorthRedState: "));
  Serial.println(pinNorthRedState);
  Serial.print(F("pinWestYellowState: "));
  Serial.println(pinWestYellowState);
  Serial.print(F("pinSouthBlueState: "));
  Serial.println(pinSouthBlueState);
  Serial.print(F("pinEastGreenState: "));
  Serial.println(pinEastGreenState); analogRead(pinPot);
  EMA_S = (EMA_a * potValue) + ((1 - EMA_a) * EMA_S); //run the EMA                         //print digital value to serial
  potValue = EMA_S * 100 / analogInputMax; // 0 - 100
  potValueAngle = map(potValue, 0, 100, (-290 / 2) + 17, (290 / 2) + 17);
  potDegreeAngle = (int)potValueAngle / 1.15 ; //  1.15 is the angle to degrees constan that I got from experimentation
  EMA_S_array[counter] = potDegreeAngle;


  if (counter == (sizeArray - 1)) {
    counter = -1;
    potRealAngle = average();
    Serial.print(F("*****************************************"));
    Serial.print(F("potRealAngle: "));
    Serial.println(potRealAngle);
  }
}
int average() {
  int aux = 0;
  for (int i = 0; i < sizeArray; i++) {
    aux = + aux + EMA_S_array[i];
  }
  return aux / sizeArray;
}
//////////////////
// interruptions /
//////////////////

void ISR_btnNorthRed() {
  moveR = true;
}
void  ISR_btnWestYellow() {
  moveforwards = true;
}

void  ISR_btnEastGreen() {
  moveback = true;
}

void ISR_btnSouthBlue() {
  //setStepmotor(-1 , 5 , 5); // (direction (1 or -1) , velocity (step/microsec) , angle)
  moveL = true;
}

void ISR_EMERGENCY() {
  //(digitalRead(EMERGENCY_BUTTON_PIN_STATE) == LOW) ? IS_EMERGENCY = true : IS_EMERGENCY = IS_EMERGENCY;
  IS_EMERGENCY = !IS_EMERGENCY;
  stopDCMotor();
  setStepmotor(-1 , 0 , 0);
  Serial.print(F("**********************"));
  Serial.print(F("IS_EMERGENCY: "));
  Serial.println(IS_EMERGENCY);
  Serial.print(F("**********************"));
}

//////////////////////////
//WEB FUNCTIONS///////////
//////////////////////////
void ProcessButton_NORTH() {

  myStepper.step(1);
  server.send(200, "text/plain", ""); //Send web page

}