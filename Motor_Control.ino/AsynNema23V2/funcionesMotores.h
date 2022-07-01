/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
  Written by BARRAGAN and modified by Scott Fitzgerald
  modified by Daniel Delgado 11/06/2022
*********/

#include <Stepper.h>
#include "Arduino.h"
/*
  // pin definitions
  #define pinLimitadorArco 1 // *** limits ***
  #define pinLimitadorCarro 3
  #define pinEnableSTPM 17 // *** STPM arc motor pins ***
  #define pinDireccionSTPM 16
  #define pinPulsosSTPM 4 // *** Car motor pins ***
  #define pinDirMasMasMotor 19
  #define pinDirMenosMenosMotor 18
  #define pinPWM_motor 21 // *** encoder pins ***
  #define pinEncoderA 23
  #define pinEncoderB 22
  #define pinAngulo 33 // *** angular sensor pin***
  #define pinCarroMas 39
  #define pinCarroMenos 36
  #define pinArcoMas 35
  #define pinArcoMenos 34
  #define pinEmergencia 2
  //pins microSD card comunnication (set by default for ESP32)
  //#define SCK  17
  //#define MISO  19
  //#define MOSI  23
  //#define CS  5

*/
//////////////////////////////////////
/////////// Definitions //////////////
//////////////////////////////////////

// esp32 constants
#define analogInputMax 4095

// pins stepper motor
#define DIR_STPM 16 //  Motor Driver Pins
#define PULSE 4
#define EN_STPM 17
//#define pinPot 32

int STEPS_PER_REV = 400;
int Angletostep = 0;
// pins  button
#define EMERGENCY_BUTTON_PIN 2
#define pinNorth 35
#define Carromas 36
#define pinSouth 39
#define Carromenos 34
# define Limcarro 25
# define Limarco 26 


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


//// pins potenciometer
//#define pinPot 35

// pins DC motor
#define pinDCMotorDriveEn 21
#define pinDCMotorDriveIN1 22
#define pinDCMotorDriveIN2 13
#define ENCA 14
#define ENCB 27

// Math constants
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105


const int car_radius_gear = 11;// mm
const int velMotorDC_max = 120;// rpm
const int radius = 800;// mm
const int N_rack = 40 * 2; // 25 * 32 number of racks by the number of theeth in each rack
const double NperArcLength = N_rack / (PI*radius);
const int unitaryStepPerDegree = STEPS_PER_REV / 360;

// Home position by default
double elActual = 90;
double azActual = 0;
double elNext = 90;// 42.05
double azNext = 0; // 297.38
double u_carActual = 0;
double u_carNext = 0;
int arcAngleActual = 0;
int arcAngleNext = 0;

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
volatile int posCarro = 0;

// stpm car variable and constants
int numOfTeethMoved = 0;
const double teethPermm = 40 / 1000;
const int diameterCarGear = 24; // mm

// booleans
bool IS_EMERGENCY = false;
//int EMERGENCY_BUTTON_PIN_STATE = HIGH; // negative logic
bool isManual = true;

int average() {
  int aux = 0;
  for (int i = 0; i < sizeArray; i++) {
    aux = + aux + EMA_S_array[i];
  }
  return aux / sizeArray;
}

void updateValues() {
  elActual = elNext;
  azActual = elNext;
  arcAngleActual = arcAngleNext;
  u_carActual = u_carNext;
}


void stopDCMotor() {
  Serial.println(F(" DC Motor stopped "));
  digitalWrite(pinDCMotorDriveIN1, LOW);
  digitalWrite(pinDCMotorDriveIN2, LOW);
}

void setMovDCMotorForward() {
  Serial.println(F(" DC motor Moving Forward "));
  digitalWrite(pinDCMotorDriveIN1, HIGH);
  digitalWrite(pinDCMotorDriveIN2, LOW);
}

void setMovDCMotorBackwards() {
  Serial.println(F(" DC motor Moving Backwards "));
  digitalWrite(pinDCMotorDriveIN1, LOW);
  digitalWrite(pinDCMotorDriveIN2, HIGH);
}

void pwmMovForward() {
  setMovDCMotorForward();
  for(int i = 0; i <= 10; i++){
    ledcWrite(dcMotorChannel, 255);
  }
  stopDCMotor();
}

void pwmMovBackwards() {
  setMovDCMotorBackwards();
  for(int i = 0; i <= 10; i++){
    ledcWrite(dcMotorChannel, 255);
  }
  stopDCMotor();
}

bool getIsManual() {
  return isManual;
}

void setIsManual(bool input) {
  isManual = input;
}

// update values


void setupMotors() {
  pinMode(pinDCMotorDriveIN1, OUTPUT);
  pinMode(pinDCMotorDriveIN2, OUTPUT);
  pinMode(pinDCMotorDriveEn, OUTPUT);
  
  //attachInterrupt(EMERGENCY_BUTTON_PIN, ISR_EMERGENCY, FALLING);
  ledcSetup(dcMotorChannel, freq, resolution);
  ledcAttachPin(pinDCMotorDriveEn, dcMotorChannel);
   // set the speed at 5 rpm
  setIsManual(true);
}

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
void Limitcarro (){
  bool limite_carro = digitalRead(Limcarro);
  if (limite_carro) {
    ledcWrite(dcMotorChannel, 0); 
  }
  
}
void Limitarco (){
  
    digitalWrite(PULSE,LOW);
    digitalWrite(EN_STPM, LOW);
  
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
int numPulsesCar() {
  u_carNext = radius * cos(elNext) * sin(azNext); // milimeters
  //d_car = sqrt(pow(radius, 2) * cos(elNext) - pow(radius, 2) * pow(cos(elNext), 2) * pow(cos(azNext), 2)) - u_carActual;
  double theta_u = asin(u_carActual / radius); //
  double theta_u_star = asin(u_carNext / radius);
  double theta_car = theta_u_star - theta_u;
  double C_car = radius * theta_car; //
  double pulsesPerN = 45 / N_rack; // hayado experimentalmente
  int N_car = (int)C_car * NperArcLength; //
  Serial.print(F("N_car: "));
  Serial.println(N_car);
  int pulses_car = N_car * pulsesPerN;
  return pulses_car;
}

int numStepsArc() {
  arcAngleNext = (int)atan2(radius * sin(elNext), radius * cos(elNext) * cos(azNext));
  return unitaryStepPerDegree * arcAngleNext;
}
