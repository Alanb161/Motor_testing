//Incluindo biblioteca "Servo.h", necessária para o controle do ESC
//ESC = Electronic Speed Controller, componente usado para controlar a velocidade dos motores elétricos BLDC
#include <Servo.h>
//Incluindo biblioteca "HX711.h", necessária para programar os amplificadores de sinal das células de carga
#include <HX711.h>
//Incluindo "stdio.h", "stdlib.h" e "math.h", bibliotecas de C++ comuns utilizadas na maioria dos programas escritos nessas linguagens
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//Incluindo "LiquidCrystal_I2C.h", necessária para programar display LCD usado para mostrar dados de sensores
#include <LiquidCrystal_I2C.h>
//Incluindo "Wire.h", necessária para fazer programação da comunicação I2C, sendo q I2C é um protocolo de comunicação entre dispositivos
#include <Wire.h>


//=========================CÓDIGO PARA CONFIGURAR O CONTROLE DE VELOCIDADE DO MOTOR FEITO POR ESC+ARDUINO===========================

//Colocando o pino de sinal do ESC no pino digital 9 do Arduino
//byte servoPin = 9;
//Definindo um objeto chamado "servo" da classe "Servo", esta que está pré-programada na biblioteca "Servo.h"
//Servo servo;

//int val = 0;
                                              // Variable for the speed sent to the ESC


//==================================================================================================================================


//===========================CÓDIGO PARA CONFIGURAR OS AMPLIFICADORES DE SINAL DAS CÉLULAS DE CARGA===================================

//Configurando pinos dos HX711(amplificadores de sinal) das células de carga

//balanca 1
#define DOUT1    4
#define CLK1   5
//balanca 2
#define DOUT2    6
#define CLK2   7
//balanca3
#define DOUT3    8
#define CLK3   9

#define l1 10   
#define l2 11
#define l3 12

// instancia Balança HX711
HX711 peso1;
HX711 peso2;
HX711 peso3;

// fator de calibração para teste das células de carga
float CF1 = (19516.27);
float CF2 = (24353.32);
float CF3 = (23820.75);


//================================================DEFINIÇÃO DE VARIÁVEIS======================================================

//definindo variáveis para torque, tração e outras grandezas relacionadas ao conjunto motor-hélice
float torque = 0;
float tracao = 0;
float acelerador = 0;

//=============================================================================================================================
//Definindo variáveis para as medições de corrente e tensão
float mVperAmp = 40;
float Corrente_offset = 2500;
float Tensao_offset = 0;
float potencia = 0;
float consumo = 0;
float eficiencia = 0;
float tensao = 0.0;               
float corrente = 0.0;
//=====================================================DEFINIÇÃO DO DISPLAY=========================================================
LiquidCrystal_I2C lcd(0x27, 20, 4);

//===================================================================================================================================
//===================================================================================================================================
//===================================================================================================================================





//FIM DA PARTE INICIAL DO PROGRAMA, INICIO DA FUNÇÃO SETUP(FUNÇÃO SEMPRE PRESENTE EM PROGRAMAS DE ARDUINO, SENDO QUE O CÓDIGO DENTRO
//DELA É EXECUTADO SOMENTE UMA VEZ



void setup() {
  
Serial.begin(9600);  //Iniciando a comunicação serial e colocando uma taxa de trasmissão de 9600 bits/s (nao sei se é bits/s)

//============================DEFINIÇÃO DE COMO SERÁ O CONTROLE DE VELOCIDADE FEITO POR ESC+ARDUINO=================================

//Parte do Setup para configurar o controle do ESC


/*
servo.attach(servoPin);  //ligando o pino de sinal do ESC ao objeto "servo" criado no início do programa

servo.writeMicroseconds(1500); //enviando sinal de parada ao ESC

delay(7000); //delay para permitir o ESC reconhecer o sinal de parada
*/
                                         // Wait for a while


//=================================================CONFIGURAÇÃO DO DISPLAY LCD======================================================
lcd.init();  // inicia o lcd-i2c
lcd.setBacklight(HIGH);  //seta a luz de blacklight para maximo brilho

//=============================FINALIZAÇÃO DA CONFIGURAÇÃO DOS AMPLIFICADORES DE SINAL DAS CÉLULAS DE CARGA=========================

// inicializa a balança
peso1.begin(DOUT1, CLK1);
peso2.begin(DOUT2, CLK2);
peso3.begin(DOUT3, CLK3);

// configura a escala da Balança
peso1.set_scale();
peso2.set_scale();
peso3.set_scale();
delay(500);

//==================================CONFIGURAÇÃO DOS SENSORES DE CORRENTE, TENSÃO ELÉTRICA E ROTAÇÃO================================

pinMode(2, INPUT); // pino do sensor de rpm como entrada
pinMode(A5, INPUT); //pino do sensor de tensão como entrada
pinMode(A0, INPUT); //pino do sensor de corrente como entrada

//=====================================CONFIGURAÇÃO INICIAL DO REGISTRO DE DADOS NO EXCEL===========================================

int LABEL = 1;
Serial.println("CLEARDATA");
Serial.println("LABEL, Data, Hora, Corrente [A], Tensão [V], Tração [N], Torque [N]");

//==================================================================================================================================

}

//===================================================================================================================================
//===================================================================================================================================
//===================================================================================================================================




//FIM DA FUNÇÃO SETUP, INICIO DA FUNÇÃO LOOP, FUNÇÃO PRESENTE EM TODO PROGRAMA DE ARDUINO, EM QUE É COLOCADO CÓDIGO QUE SERÁ EXECUTADO
//REPETIDAS VEZES CONTINUAMENTE ATÉ QUE A ALIMENTAÇÃO SEJA DESCONECTADA OU O PROGRAMA SEJA PARADO 




void loop() {

//==============================DEFINIÇÃO DE COMO SERÁ O CONTROLE DE VELOCIDADE FEITO POR ESC+ARDUINO===============================

//Programação para controlar o ESC com sinal do tipo PWM


Serial.println("Enter PWM signal value 1100 to 1900, 1500 to stop");

while (Serial.available() == 0);

val = Serial.parseInt();

if (val < 1100 || val > 1900)
{
  Serial.println("not valid");
}
else
{
  servo.writeMicroseconds(val); //send signal to ESC
}


//==============================RECEBIMENTO DE DADOS DOS SENSORES DE TENSÃO ELÉTRICA E CORRENTE=====================================

//medição de corrente na porta analógica A5, tomando-se 1000 medidas com intervalos de 1 milisegundo entre si e fazendo a sua média

float voltage_average = 0;
for(int j = 0; j < 1000; j++)
{
  voltage_average = voltage_average + analogRead(A5)/1000;
}
tensao = map(voltage_average, 0, 1023, 0 ,2500) + Tensao_offset;
tensao = tensao/100;

//medição de corrente na porta analógica A0, tomando-se 1000 medidas com intervalos de 1 milisegundo entre si e fazendo a sua média
float current_average = 0;
for(int i = 0; i < 1000; i++)
{
  current_average = current_average + (0.44*analogRead(A0)-3.78)/1000;
  delay(1);
}
corrente = current_average;

//========================================RECEBIMENTO DE DADOS DAS CÉLULAS DE CARGA=============================================

// ajusta fator de calibração
peso1.set_scale(CF1);
peso2.set_scale(CF2);
peso3.set_scale(CF3);

tracao = peso1.get_units();
torque = (abs(peso3.get_units()+peso2.get_units())/16.48);

//===================================PRINTA OS DADOS NO MONITOR SERIAL E NO DISPLAY LCD=========================================

lcd.clear();  // limpa o lcd
lcd.setCursor(0, 0);  // seta o cursor para a posiçao 0,0
lcd.print("C1:");                       // imprime a grandeza mensurada
lcd.setCursor(2, 0);   
lcd.print(peso1.get_units(), 4);         // imprime a tração com 2 casas decimais
 
lcd.setCursor(0, 1);
lcd.print("C2:");                       
lcd.setCursor(2, 1);  
lcd.print(peso2.get_units(), 4);         // imprime o torque levando em consideração o binario de forças das celulas de carga pela distancia

lcd.setCursor(0, 2);
lcd.print("C3:");                       
lcd.setCursor(2, 2);  
lcd.print(peso3.get_units(), 4);

//printa potencia mecanica no lcd
lcd.setCursor(11,0);
lcd.print("V:");
lcd.setCursor(13,0);
lcd.print(tensao, 3);
   
//printa a potencia eletrica no lcd
lcd.setCursor(11,1);
lcd.print("A:");
lcd.setCursor(13,1);
lcd.print(corrente, 3);

//=============================================IMPRESSÃO DOS VALORES DOS SENSORES NO EXCEL===================================================     
  
//imprime para a serial
Serial.print("DATA,DATE,TIME,");
Serial.print(corrente);
Serial.print(",");
Serial.print(tensao);
Serial.print(",");
Serial.print(tracao);
Serial.print(",");
Serial.print(torque);
Serial.print(",");
Serial.println("SCROLLDATA_20");
    
}  

//==================================================================================================================================
//===================================================================================================================================
//===================================================================================================================================


//FIM DO PROGRAMA - AQUI SERÃO COLOCADOS ALGUNS CÓDIGOS ADICIONAIS ÚTEIS

//Código para testar se as células de carga estão funcionando e os módulos amplificadores não estão queimados

/*
#include "HX711.h"

// HX711 circuit wiring

const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 4;

HX711 scale;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {

  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found.");
  }

  delay(1000);
  
}
*/
