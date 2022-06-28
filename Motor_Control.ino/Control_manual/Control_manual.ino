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

// setting PWM_STPM properties
const int freq2 = 5000;
const int MotorChannel2 = 1;
const int resolution2 = 14;
int max_duty_cycle2 = (int)(pow(2, resolution) - 1);
float wait2 = (1.0 / max_duty_cycle) * 1000.0;



bool moveR = false;
bool moveL =  false;
bool moveforwards = false;
bool moveback = false;
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
  if(moveforwards){
  digitalWrite(pinDCMotorDriveIN1, LOW ); //Switch between this HIGH and LOW to change direction
  digitalWrite(pinDCMotorDriveIN2,  HIGH);
  ledcWrite(dcMotorChannel, 180);
  delay(100);
    digitalWrite(pinDCMotorDriveIN1, LOW );
  digitalWrite(pinDCMotorDriveIN2,  LOW);
  moveforwards =false;
  }
   if(moveback){
    digitalWrite(pinDCMotorDriveIN1, HIGH); //Switch between this HIGH and LOW to change direction
  digitalWrite(pinDCMotorDriveIN2, LOW);
  ledcWrite(dcMotorChannel, 180);
  delay(100);
  digitalWrite(pinDCMotorDriveIN1, LOW); //STOP
  digitalWrite(pinDCMotorDriveIN2, LOW);
   moveback =false;
}
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
     moveR= true;
}
void  ISR_btnWestYellow() {
      moveforwards=true;
}

void  ISR_btnEastGreen() {
      moveback=true;
}

void ISR_btnSouthBlue() {
  //setStepmotor(-1 , 5 , 5); // (direction (1 or -1) , velocity (step/microsec) , angle)
   moveL=true;
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
