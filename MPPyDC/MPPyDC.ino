/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
  Written by BARRAGAN and modified by Scott Fitzgerald
  modified by Daniel Delgado 11/06/2022
*********/

#include <Servo.h>
#include <Stepper.h>

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
// Definitions
#define IN1 26 // ULN2003 Motor Driver Pins
#define IN2 25
#define IN3 33
#define IN4 32
#define pinDcMotorPWM 12 // define pin of data of the servo
#define pinJoystickX 4
#define pinJoystickY 15

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

bool isManual = true;
int joystickXvalue = 0;
int joystickYvalue = 0;
int mapX = 0;
int mapY = 0;


Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // initialize the stepper library

void setup() {
  ledcSetup(dcMotorChannel, freq, resolution);
  ledcAttachPin(pinDcMotorPWM, dcMotorChannel);

  myStepper.setSpeed(5); // set the speed at 5 rpm
  Serial.begin(115200); // initialize the serial port
  Serial.println("Motor code");

  setIsManual(true);


}

void loop() {

  if (getIsManual()) {
    manualMov();
  } else {
    automaticMov(42.05, 297.38);
  }
}


// Type of movement
void manualMov() {
  int joystick_offset = 150;
  // offset jaystick -> mapX: 78 mapY: 95
  joystickXvalue = analogRead(pinJoystickX);
  joystickYvalue = analogRead(pinJoystickY);
  mapX = map(joystickXvalue, 0, 4095, -255, 255);
  Serial.print(F("mapX: "));
  Serial.print(mapX);
  if ((mapX > joystick_offset) || (mapX < -joystick_offset)) {
    pwmMov(mapX);
  }
  mapY = map(joystickYvalue, 0, 4095, -255, 255);
  Serial.print(F(" mapY: "));
  Serial.println(mapY);
  if (mapY > joystick_offset) {
    motorArcMov(1);
  }
  if (mapY < -joystick_offset) {
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
}

// motors movement
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

// arc and stepper functions
void arcAngleNextCalc() {
  arcAngleNext = (int)atan2(radius * sin(elNext), radius * cos(elNext) * cos(azNext));
}

int numStepsArc() {
  return unitaryStepPerDegree * arcAngleNext;
}

// car and dc motor functions
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
  ledcWrite(dcMotorChannel, dutyCycle);
  delay(15);
}

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
