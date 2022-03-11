#include "HX711.h"

// definição dos pinos das células de carga
//balanca 1
#define DOUT1    3
#define CLK1   4
//balanca 2
#define DOUT2    5
#define CLK2   6
//balanca3
#define DOUT3    7
#define CLK3   8

#define l1 10   
#define l2 11
#define l3 12

//fatores de calibração
float CF1 = (19516.27);
float CF2 = (24353.32);
float CF3 = (23820.75);
HX711 peso1;
HX711 peso2;
HX711 peso3;

HX711 scale;
int pulsos = 0;
int rpm = 0;
int timeold = 0;
int pulsos_por_volta = 2;
int contador = 0;

void setup() {

//inicializando células de carga
Serial.begin(9600);
peso1.begin(DOUT1, CLK1);
peso2.begin(DOUT2, CLK2);
peso3.begin(DOUT3, CLK3);
}

void loop() {

//aplicando os fatores de calibração
peso1.set_scale(CF1);
peso2.set_scale(CF2);
peso3.set_scale(CF3);

Serial.print("=================");
Serial.print("C1:");                       
Serial.print(peso1.get_units(), 4);         // imprime a tração com 2 casas decimais
Serial.print("================="); 

Serial.print("C2:");                        
Serial.print(peso2.get_units(), 4);         // imprime o torque levando em consideração o binario de forças das celulas de carga pela distancia
Serial.print("=================");

Serial.print("C3:");                         
Serial.print(peso3.get_units(), 4);
Serial.print("=================");

delay(1000);


//código do sensor de rotação, ainda não funcionando
if (millis() - timeold >= 1000) //utiliza a funcao millis()
{
 //Desabilita interrupcao durante o calculo
 detachInterrupt(0);
 int pulsos_por_volta = 2;
 rpm = (60 * 1000 / pulsos_por_volta ) / (millis() - timeold) * pulsos;
 timeold = millis();
 pulsos = 0;

 //Mostra o valor de RPM no serial monitor
 Serial.print("RPM = ");
 Serial.println(rpm, DEC);
 //Habilita interrupcao
 attachInterrupt(0, contador, RISING);
}
  
}
