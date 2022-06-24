#include "Arduino.h"

// pins stepper motor
#define DIR_STPM 16 //  Motor Driver Pins
#define PULSE 4 
#define EN_STPM 17
#define pinPot 32

int STEPS_PER_REV = 400;
int Angletostep = 0;
// pins  button
#define EMERGENCY_BUTTON_PIN 2
#define pinNorth 35
#define pinWest 36
#define pinSouth 39
#define pinEast 34


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

// potenciometer

volatile double potValue = 0;
double maxPotAngle = 290; // degrees
volatile double pot_offset = 0; // offset to set zero
volatile double potValueAngle = 0;
double potValueAngle_offset = 0;
volatile int potDegreeAngle = 0;
float EMA_a = 0.6;      //initialization of EMA alpha
int EMA_S = 0;          //initialization of EMA S
const int sizeArray = 2;
int EMA_S_array[sizeArray];
int counter = 0;
volatile int potRealAngle = 0;

bool IS_EMERGENCY = false; // EMERGENCY
int posCarro = 0; // encoder
volatile int pinNorthRedState = HIGH;
volatile int pinWestYellowState = HIGH;
volatile int pinSouthBlueState = HIGH;
volatile int pinEastGreenState = HIGH;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  pinMode(ENCA,INPUT);
  pinMode(ENCB,INPUT);  
    pinMode(pinDCMotorDriveIN1, OUTPUT);
  pinMode(pinDCMotorDriveIN2, OUTPUT);
  pinMode(pinNorth, INPUT_PULLUP);
  pinMode(pinWest, INPUT_PULLUP);
  pinMode(pinSouth, INPUT_PULLUP);
  pinMode(pinEast, INPUT_PULLUP);
  attachInterrupt(pinNorth, ISR_btnNorthRed, FALLING);
  attachInterrupt(pinWest, ISR_btnWestYellow, FALLING);
  attachInterrupt(pinSouth, ISR_btnSouthBlue, FALLING);
  attachInterrupt(pinEast, ISR_btnEastGreen, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCA),readEncoder,RISING);
  ledcSetup(dcMotorChannel, freq, resolution);
  ledcAttachPin(pinDCMotorDriveEn, dcMotorChannel);
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
manualMov();
}
//////////////////
// Motor Movement /
//////////////////


void setStepmotor(int dirPin, int Duty, int target){
  if (dirPin == 1){
    // Set motor direction clockwise
    Angletostep = (STEPS_PER_REV / 360)*target;
     digitalWrite (EN_STPM,HIGH);
      for(int x = 0; x < Angletostep ; x++) {  // define CCW or CW rotation
        digitalWrite(PULSE,LOW);
        delayMicroseconds(Duty);
        digitalWrite(PULSE,HIGH);
        delayMicroseconds(Duty);
      }
  }
      else {
   // Set motor direction counterclockwise
      for(int x = 0; x < Angletostep ; x++) { // define CCW or CW rotation
        digitalWrite(PULSE,HIGH);
        delayMicroseconds(Duty);
        digitalWrite(PULSE,LOW);
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
  
}
//////////////////
// Sensors /
//////////////////
void readEncoder(){
  int b = digitalRead(ENCB); // Calcula la posición en Grados del motor (verificar PPR)
  if(b > 0){
    posCarro++;
  }
  else{
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
  setStepmotor(1 , 5 , 5);  // (direction (1 or -1) , velocity (microsec/step) -> Rising value goes slowly , angle)
}
void  ISR_btnWestYellow() {
  digitalWrite(pinDCMotorDriveIN1, LOW ); //Switch between this HIGH and LOW to change direction
  digitalWrite(pinDCMotorDriveIN2,  HIGH);
  ledcWrite(dcMotorChannel,50);
  Serial.println("Interrupcion Verde");
}
void ISR_btnSouthBlue() {
  setStepmotor(-1 , 5 , 5); // (direction (1 or -1) , velocity (step/microsec) , angle)
}
void  ISR_btnEastGreen() {
  digitalWrite(pinDCMotorDriveIN1, HIGH); //Switch between this HIGH and LOW to change direction
  digitalWrite(pinDCMotorDriveIN2, LOW);
  ledcWrite(dcMotorChannel,50);
  Serial.println("Interrupcion azul");
}

void ISR_EMERGENCY() {
  //(digitalRead(EMERGENCY_BUTTON_PIN_STATE) == LOW) ? IS_EMERGENCY = true : IS_EMERGENCY = IS_EMERGENCY;
  IS_EMERGENCY = true;
  stopDCMotor();
  setStepmotor(-1 , 0 , 0);
  Serial.print(F("**********************"));
  Serial.print(F("IS_EMERGENCY: "));
  Serial.println(IS_EMERGENCY);
  Serial.print(F("**********************"));
}
