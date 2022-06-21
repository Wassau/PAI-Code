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
#define IN1 26 // ULN2003 Motor Driver Pins
#define IN2 25
#define IN3 33
#define IN4 32

// pins joystick
#define pinJoystickX 4
#define pinJoystickY 15

// pins potenciometer
#define pinPot 35
#define pinCalibration 16

// pins  button
#define EMERGENCY_BUTTON_PIN 22
#define pinGreenBtn 21
#define BUZZER_PIN 27

// pins DC motor
#define pinDCMotorDriveEn 13
#define pinDCMotorDriveIN1 12
#define pinDCMotorDriveIN2 14

// Math constants
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

// Motors constants
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// setting PWM properties
const int freq = 5000;
const int dcMotorChannel = 0;
const int resolution = 8;
int max_duty_cycle = (int)(pow(2, resolution) - 1);
float wait = (1.0 / max_duty_cycle) * 1000.0;

const int car_radius_gear = 11;// mm
const int velMotorDC_max = 120;// rpm
const int radius = 800;// mm
const int N_rack = 40 * 2; // 25 * 32 number of racks by the number of theeth in each rack
const double NperArcLength = N_rack / (PI*radius);
const int unitaryStepPerDegree = stepsPerRevolution / 360;

// Home position by default
double elActual = 90;
double azActual = 0;
double elNext = 90;// 42.05
double azNext = 0; // 297.38
double u_carActual = 0;
double u_carNext = 0;
int arcAngleActual = 0;
int arcAngleNext = 0;


// Location to move
const double latitude = 4.637;
const double longitude = -74.083;

// joystick
bool isManual = true;
int joystickXvalue = 0;
int joystickYvalue = 0;
int mapX = 0;
int mapY = 0;

// potenciometer
volatile double potValue = 0;
double maxPotAngle = 290; // degrees
volatile double pot_offset = 0; // offset to set zero
volatile double potValueAngle = 0;
double potValueAngle_offset = 0;
volatile int potDegreeAngle = 0;
float EMA_a = 0.6;      //initialization of EMA alpha
int EMA_S = 0;          //initialization of EMA S
const int sizeArray = 10;
int EMA_S_array[sizeArray];
int counter = 0;
volatile int potRealAngle = 0;

// stpm car variable and constants
int numOfTeethMoved = 0;
const double teethPermm = 40 / 1000;
const int diameterCarGear = 24; // mm

// EMERGENCY
bool IS_EMERGENCY = false;
int EMERGENCY_BUTTON_PIN_STATE = HIGH; // negative logic
bool isCalibrated = false;
int greenBtnState = HIGH; // negative logic
bool calibrationLedState = LOW;


Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // initialize the stepper library


//*******************************************
//***************** MAIN ********************
//*******************************************


void setup() {
  Serial.begin(115200); // initialize the serial port
  pinMode(pinCalibration, OUTPUT);
  pinMode(pinDCMotorDriveIN1, OUTPUT);
  pinMode(pinDCMotorDriveIN2, OUTPUT);
  pinMode(pinGreenBtn, INPUT_PULLUP);
  attachInterrupt(EMERGENCY_BUTTON_PIN, ISR_EMERGENCY, FALLING);
  ledcSetup(dcMotorChannel, freq, resolution);
  ledcAttachPin(pinDCMotorDriveEn, dcMotorChannel);
  myStepper.setSpeed(5); // set the speed at 5 rpm
  setIsManual(true);


}

void loop() {
  if (!IS_EMERGENCY) {
    while (!isCalibrated) {
      isCalibratedUpdate();
    }
    potToAngleUpdate();
    if (counter == -1) {
      if (getIsManual()) {
        manualMov();
      } else {
        automaticMov(42.05, 297.38);
      }
    }

  }
  counter++;
}

//*******************************************
//*************** FUNCTIONS *****************
//*******************************************

void potToAngleUpdate() {
  potValue = analogRead(pinPot);
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

void isCalibratedUpdate() {
  (digitalRead(pinGreenBtn) == LOW) ? isCalibrated = true : isCalibrated = isCalibrated;
  pot_offset = analogRead(pinPot);
  Serial.print(F("pot_offset NEW: "));
  Serial.println(pot_offset);
  if (pot_offset > 1050 && pot_offset < 1080) {
    Serial.print(F("calibrationLedState NEW: "));
    Serial.println(calibrationLedState);
    calibrationLedState = HIGH;
    digitalWrite(pinCalibration, calibrationLedState);
  } else {
    calibrationLedState = LOW;
    digitalWrite(pinCalibration, calibrationLedState);
  }
}

int average() {
  int aux = 0;
  for (int i = 0; i < sizeArray; i++) {
    aux = + aux + EMA_S_array[i];
  }
  return aux / sizeArray;
}
/////////////////////////////////////////////
//////////// Type of movement ///////////////
/////////////////////////////////////////////
void manualMov() {
  int joystick_offset = 150;
  // offset jaystick -> mapX: 78 mapY: 95
  joystickXvalue = analogRead(pinJoystickX);
  joystickYvalue = analogRead(pinJoystickY);
  mapX = map(joystickXvalue, 0, analogInputMax, (-analogInputMax - 800 / 2 ), (analogInputMax - 800 / 2 ));
  Serial.print(F("mapX: "));
  Serial.print(mapX);
  if ((mapX > joystick_offset) || (mapX < -joystick_offset)) {
    if (numOfTeethMoved < 30 && numOfTeethMoved > -30) {
      unsigned long time_o = millis() ;
      pwmMov((255 / 4) * mapX / (analogInputMax / 2));
      unsigned long time_f = millis() ;
      //numOfTeethMovedUpdate(time_o, time_f, (255 / 4) * mapX / (analogInputMax / 2));
    }
  }
  mapY = map(joystickYvalue, 0, analogInputMax, -255, 255);
  Serial.print(F(" mapY: "));
  Serial.println(mapY);
  if (mapY > joystick_offset && potRealAngle <= 80) { //&& encoderValue <= 4 North to south is positive
    motorArcMov(1);
  }
  if (mapY < -joystick_offset && potRealAngle >= -80) { // && encoderValue >= -4
    motorArcMov(-1);
  }
}


void automaticMov(float elNextInput, float azNextInput) {
  setNextPos(elNextInput, azNextInput);
  motorCarMov();
  delay(1000);

  motorArcMov(numStepsArc());
  delay(1000);

  showPos();
  delay(1000);

  Serial.print(F("Acomoda el arco en su posición de incio y oprime el botón"));

}
/////////////////////////////////////////////
//////// motors movement functions //////////
/////////////////////////////////////////////
void motorCarMov() {
  if ((u_carNext < 600) && (u_carNext > -600)) {
    // function of car movement
    updateValues();
  }

}
void motorArcMov(int numSteps) {
  arcAngleNextCalc();
  myStepper.step(numSteps);
  updateValues();
  if ((arcAngleNext < 80) && (arcAngleNext > -80)) {
    //
  }
}

/////////////////////////////////////////////
/////// arc and stepper functions ///////////
/////////////////////////////////////////////
void arcAngleNextCalc() {
  arcAngleNext = (int)atan2(radius * sin(elNext), radius * cos(elNext) * cos(azNext));
}

int numStepsArc() {
  return unitaryStepPerDegree * arcAngleNext;
}



/////////////////////////////////////////////
/////// car and dc motor functions //////////
/////////////////////////////////////////////
int numPulsesCar() {
  u_carNext = radius * cos(elNext) * sin(azNext); // milimeters
  //d_car = sqrt(pow(radius, 2) * cos(elNext) - pow(radius, 2) * pow(cos(elNext), 2) * pow(cos(azNext), 2)) - u_carActual;
  double theta_u = asin(u_carActual / radius); //
  double theta_u_star = asin(u_carNext / radius);
  double theta_car = theta_u_star - theta_u;
  double C_car = radius * theta_car; //

  int N_car = (int)C_car * NperArcLength; //
  int pulses_car = N_car * pulsesPerN();
  return pulses_car;
}

int pulsesPerN() {
  double revPerPulse = PI / (30 * freq);
  double numPulsesTotalArc = (PI * radius) / (TWO_PI * car_radius_gear * revPerPulse);
  int numPulsesPerN = (int)numPulsesTotalArc / N_rack;
  return numPulsesPerN;
}

void pwmMov(int dutyCycle) {
  digitalWrite(pinDCMotorDriveIN1, HIGH);
  digitalWrite(pinDCMotorDriveIN2, HIGH);
  (dutyCycle > 0) ? setMovDCMotorForward() : setMovDCMotorBackwards();
  ledcWrite(dcMotorChannel, abs(dutyCycle));
  delay(15);
}

void numOfTeethMovedUpdate(unsigned long time_o, unsigned long time_f, int dutyCycle) {
  unsigned long deltat = time_f - time_o;//14
  Serial.print(F("delta de tiempo: "));
  Serial.println(deltat);
  double omega = (dutyCycle) / 255 * velMotorDC_max;
  Serial.print(F("omega: "));
  Serial.println(omega);
  double vel = PI * diameterCarGear * omega / 60; // mm/s
  Serial.print(F("velocidad: "));
  Serial.println(vel);
  numOfTeethMoved = + (int)teethPermm * vel * deltat;
  Serial.print(F("numOfTeethMoved: "));
  Serial.println(numOfTeethMoved);
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


/////////////////////////////////////////////
//////////// auxiliar functions /////////////
/////////////////////////////////////////////
void setNextPos(float elNextInput, float azNextInput) {
  elNext = elNextInput;// 42.05
  azNext = azNextInput; // 297.38
}

bool isHome() {
  return ((elActual == 90) && (azActual == 0));
}

bool getIsManual() {
  return isManual;
}

void setIsManual(bool input) {
  isManual = input;
}

// update values
void updateValues() {
  elActual = elNext;
  azActual = elNext;
  arcAngleActual = arcAngleNext;
  u_carActual = u_carNext;
}

// interruptions
void ISR_EMERGENCY() {
  //(digitalRead(EMERGENCY_BUTTON_PIN_STATE) == LOW) ? IS_EMERGENCY = true : IS_EMERGENCY = IS_EMERGENCY;
  IS_EMERGENCY = true;
  stopDCMotor();
  myStepper.setSpeed(0);
  Serial.print(F("**********************"));
  Serial.print(F("IS_EMERGENCY: "));
  Serial.println(IS_EMERGENCY);
  Serial.print(F("**********************"));
  delay(10000);
}



// show values in serial monitor
void showPos() {
  Serial.print(F("Elevación actual:"));
  Serial.print(elActual);
  Serial.print(F(" Azimut actual: "));
  Serial.print(azActual);
  Serial.print(F(" Elevación siguiente: "));
  Serial.print(elNext);
  Serial.print(F(" Azimut siguiente: "));
  Serial.print(azNext);
  Serial.print(F(" u_carActual: "));
  Serial.print(u_carActual);
  Serial.print(F(" u_carNext: "));
  Serial.println(u_carNext);
}
