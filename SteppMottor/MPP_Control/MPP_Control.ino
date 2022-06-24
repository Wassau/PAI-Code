
// DC MOTOR PINS
#define ENCB 22
#define PWM_pin 21
#define dirmas 19
#define dirmenos 18


// Definitions Step Motor
#define Enable 17 // Motor Driver Pins
#define DIRSTPM 16
#define PULSE 4
#define IN4 32
// PWM Properties.
const int freq = 30000;
const int Channel = 0;
const int resolution = 8;
int dutyCycle = 200;
int pos = 0; // Posición encoder

const int STEPS_PER_REV = 200;
int dirPin = 0;
int prevT =0;
int posi = 0;
int target = 100;
int eprev =0;
int deltaT = 0;
float eintegral =0;
void setup() {
  // put your setup code here, to run once:
    ledcSetup( Channel, freq, resolution); // set the PWM parameters 
  ledcAttachPin(PWM_pin, Channel); // assign the Pin for PWM_pin

}

void loop() {
  // put your main code here, to run repeatedly:
  
  // set target position
  int target = Serial.read();

  
  // PID constants
  float kp = 5.8;
  float kd = 3.9;
  float ki = 0.33;

  // time difference
  long currT = micros();
  float deltaT = ((float) (currT - prevT))/( 1.0e6 );
  prevT = currT;

  int pos = 0; 
    pos = posi;
  
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

  // signal to the motor
  setmotor(dir,pwr);


  // store previous error
  eprev = e;
  int pasos =  target/ 1.8;
  Serial.print(target);
  Serial.print(" ");
  Serial.print(pos);
  Serial.println();
}
  void setmotor(int dirPin, int Duty){
    if (dirPin == 1){
      // Set motor direction clockwise
        for(int x = 0; x < STEPS_PER_REV ; x++) { // one rotation
          digitalWrite(PULSE,LOW);
          delayMicroseconds(Duty);
          digitalWrite(PULSE,HIGH);
          delayMicroseconds(Duty);
        }
    }
        else {
     // Set motor direction counterclockwise
        for(int x = 0; x < STEPS_PER_REV ; x++) { // one rotation
          digitalWrite(PULSE,HIGH);
          delayMicroseconds(Duty);
          digitalWrite(PULSE,LOW);
          delayMicroseconds(Duty);
        }
    }
   }
