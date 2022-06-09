#include <util/atomic.h> 

int pin_salida=6;
int pin_entrada=2; //Encoder A
int pin_entradaB=3; //Encoder B
double referencia=0.0;
volatile int posi = 0;

int pwm =20;
double salida=0;
double entrada=0;
double entrada2=0;
double salida2=0;
double error=0;
float integral_e = 0.0;
double u = 0.0;
double t=0.0;
unsigned long tiempoInicial=millis();
float kp= 5;
double ki = 2;
int dt = 10.000;  
int pwmconvers= 255/5;


void setup() {
  Serial.begin(9600);
  pinMode(pin_salida,OUTPUT);
  pinMode(pin_entrada,INPUT);
   pinMode(pin_entradaB,INPUT);
   attachInterrupt(digitalPinToInterrupt(pin_entrada),readEncoder,RISING);
}

void loop() {
  int target = Serial.read();
  entrada=analogRead(pin_entrada)/204.6; // 1023/5 = 204.6
  entrada2=analogRead(pin_entradaB)/204.6;
 

  integral_e = integral_e +(error*10)/1000;
  int pos = 0; 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pos = posi;
    
  }
    
  u= (kp*error) +(ki*integral_e);
  
  if(u >= 5){
    u = 5;
  }else{
    u = 5;
  }
   error= pos - target;
  analogWrite(pin_salida,u*255/5);
  digitalWrite(9,HIGH);
  //t=(millis()-tiempoInicial)/1000.0;
  /*Serial.print(entrada*5);
  Serial.print(",");
  Serial.println(entrada2*5);*/
  Serial.print(u*255/5);
  Serial.print(",");
  Serial.println(pos);
   delay(10);
}
void readEncoder(){
  int b = digitalRead(pin_entradaB);
  if(b > 0){
    posi++;
  }
  else{
    posi--;
  }
}
