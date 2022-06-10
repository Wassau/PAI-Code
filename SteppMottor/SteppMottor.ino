#include <Stepper.h>
#include <util/atomic.h> 
// Define Constants
 
// Number of steps per output rotation
const int STEPS_PER_REV = 200;
const int SPEED_CONTROL = A0;
const int dirPin = 2; 
// Create Instance of Stepper Class
// Specify Pins used for motor coils
// The pins used are 8,9,10,11 
// Connected to L298N Motor Driver In1, In2, In3, In4 
// Pins entered in sequence 1-2-3-4 for proper step sequencing
 
Stepper Nema23(STEPS_PER_REV, 8, 9, 10, 11);
 
void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(pin_entrada),readEncoder,RISING);
}
 
void loop() {
  // read the sensor value:
  void loop() {

  // set target position
  //int target = 1200;
  int target = Serial.read();

  
  // PID constants
  float kp = 6.8;
  float kd = 3.9;
  float ki = 0.33;

  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  int pos = 0; 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
  }
  
  // error
  int e = pos - target;

  // derivative
  float dedt = (e-eprev)/(deltaT);

  // integral
  eintegral = eintegral + e*deltaT;

  // control signal
  float u = kp*e + kd*dedt + ki*eintegral;

  // motor power
  float pwr = fabs(u);
  if( pwr > 255 ){
    pwr = 255;
  }

  // motor direction
  int dir = 1;
  if(u<0){
    dir = -1;
  }

  // signal the motor
  setMotor(dir,pwr,PWM,dirpin);


  // store previous error
  eprev = e;
  int pasos =  target/ 1.8;
  Serial.print(target);
  Serial.print(" ");
  Serial.print(pos);
  Serial.println();
}
  int sensorReading = analogRead(SPEED_CONTROL);
  // map it to a range from 0 to 100:
  int motorSpeed = map(sensorReading, 0, 1023, 0, 100);
  // set the motor speed:
  if (motorSpeed > 0) {
    Nema23.setSpeed(motorSpeed); // PWM
    // step 1/100 of a revolution:
    Nema23.step(STEPS_PER_REV / 100); 
  }
}
void setMotor(int dir, int pwmVal, int pasos, int dirpin){
  Nema23.setSpeed(pwmVal);
  Nema23.step(pasos); 
  if(dir == 1){
      // Set motor direction clockwise
    digitalWrite(dirPin,HIGH); 
  }
  else if(dir == -1){
      // Set motor direction counterclockwise
    digitalWrite(dirPin,LOW); 
  } 
}