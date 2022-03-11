#include <Servo.h>
//Colocando o pino de sinal do ESC no pino digital 9 do Arduino
byte servoPin = 9;
//Definindo um objeto chamado "servo" da classe "Servo", esta que está pré-programada na biblioteca "Servo.h"
Servo servo;

float tensao = 0;
float corrente = 0;
float Tensao_offset = 0;

int val = 0;
void setup() {
  pinMode(A5, INPUT); //pino do sensor de tensão como entrada
  pinMode(A0, INPUT); //pino do sensor de corrente como entrada
  
  Serial.begin(9600);
  servo.attach(servoPin, 1300, 1900);  //ligando o pino de sinal do ESC ao objeto "servo" criado no início do programa

  //servo.writeMicroseconds(1600); //enviando sinal de parada ao ESC

  delay(7000); //delay para permitir o ESC reconhecer o sinal de parada

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Enter PWM signal value 1100 to 1900, 1500 to stop");
  
  Serial.println(servo.readMicroseconds());
  
  while (Serial.available() == 0);
  
  val = Serial.parseInt();

  if (val < 1200 || val > 1900)
  {
    Serial.println("not valid");
  }
  else{
    servo.writeMicroseconds(val); //send signal to ESC
  }

  
  float voltage_average = 0;
  for(int j = 0; j < 1000; j++)
  {
    voltage_average = voltage_average + analogRead(A5)/1000;
  }
  tensao = map(voltage_average, 0, 1023, 0 ,2500) + Tensao_offset;
  tensao = tensao/100.0;
  Serial.println("Tensão:\n");
  Serial.print(tensao);
  
  //medição de corrente na porta analógica A0, tomando-se 1000 medidas com intervalos de 1 milisegundo entre si e fazendo a sua média
  float current_average = 0;
  for(int i = 0; i < 1000; i++)
  {
    current_average = current_average + (0.44*analogRead(A0)-3.78)/1000;
    delay(1);
  }
  corrente = current_average;
  Serial.println("Corrente:\n");
  Serial.print(corrente);
}
