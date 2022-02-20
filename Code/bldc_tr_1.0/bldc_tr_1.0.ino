#include <HX711_ADC.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Wire.h>

//================================Sensores diversos====================================================
//Tensao
float tensao_raw;
float Tensao;


//Corrente

float Corrente;
const int mVporAmp = 40;
const float ACS_offset = 2512.22;
float corrente_raw =0;
float C_Voltage = 0;

//Variaveis
float Torque =0;
float Tracao =0;
float Consumo=0;
//PWM
float PWM;
float Acelerador;


//LED's
int LED_erro = 12;
int LED_ok = 13;

//Termistor
int ThermistorPin = 1;
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;



//===================================================================RPM=================================================================================================
const byte rpmInputPin = 2;
const byte rpmMeasurementCycles = 2;
const byte printInterval = 500;

volatile unsigned long startTime, stopTime;
volatile byte rpmCount, testState;
unsigned long rpmPeriod, currentMillis, previousMillis = 0;;
float rpmFrequency, rpm;

//==========================================================================  CELULAS DE CARGA  =========================================================================================

//Pinos celulas de carga:
const int Celula1_DT = 3;
const int Celula1_SCK = 4;

const int Celula2_DT = 5;
const int Celula2_SCK = 6;

const int Celula3_DT = 7;
const int Celula3_SCK = 8;
//HX711 
HX711_ADC LoadCell_1(Celula1_DT, Celula1_SCK); //Célula 1
HX711_ADC LoadCell_2(Celula2_DT, Celula2_SCK); //Célula 2
HX711_ADC LoadCell_3(Celula3_DT, Celula3_SCK); //Célula 3

const int calVal_eepromAdress_1 = 0; // endereco eeprom fator de calibracao 1 (4 bytes)
const int calVal_eepromAdress_2 = 4; // endereco eeprom fator de calibracao 2 (4 bytes)
const int calVal_eepromAdress_3 = 8; // endereco eeprom fator de calibracao 3 (4 bytes)
//=======================================================================================================================================================================================
  

void setup() {
  Serial.begin(9600);
  
  
  pinMode(rpmInputPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(rpmInputPin), rpmMeasure, RISING);
/*
  Serial.println();
  Serial.print("Iniciando celulas de carga");
  delay(550);
  Serial.print(".");
  delay(550);
  Serial.print(".");
  delay(550);
  Serial.print(".");
  Serial.println();
*/
//==========================================================================CELULAS DE CARGA===========================================================================================
  //Inicia as celulas:
  LoadCell_1.begin();
  LoadCell_2.begin();
  LoadCell_3.begin();

  //Variaveis celulas:
  float fatorC1; //fator de calibracao celula 1
  float fatorC2; //fator de calibracao celula 2
  float fatorC3; //fator de calibracao celula 3

//////////FATOR DE CALIBRACAO DAS CELULAS//////////
  fatorC1 = 20400;
  fatorC2 = 20000;
  fatorC3 = 26427;


  
//#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch the value from eeprom
//#endif
  
  
  //EEPROM.get(calVal_eepromAdress_1, fatorC1); // descomentar para gravar os valores na eeprom
  //EEPROM.get(calVal_eepromAdress_2, fatorC2); // descomentar para gravar os valores na eeprom
  //EEPROM.get(calVal_eepromAdress_3, fatorC3); // descomentar para gravar os valores na eeprom
  
  
  long stabilizingtime = 3000; // tempo para tarar, quanto maior melhor
  boolean _tare = true; //para tarar na proxima etapa, deixar como false
 LoadCell_1.start(stabilizingtime, _tare);
 LoadCell_2.start(stabilizingtime, _tare);
 LoadCell_3.start(stabilizingtime, _tare);

 
 //VERIFICA CELULAS DE CARGA:
 if (LoadCell_1.getTareTimeoutFlag()) {
    digitalWrite(LED_erro, HIGH);
    
    while (1);
  }
  else {
    LoadCell_1.setCalFactor(fatorC1); // aplica fato de calibracao
    delay(200);
    //Serial.println("Celula 1, OK");
  }
    if (LoadCell_2.getTareTimeoutFlag()) {
    //Serial.println("ERRO Celula 2, favor verificar."); 
    
    while (1);
  }
  else {
    LoadCell_2.setCalFactor(fatorC2); // aplica fato de calibracao
    delay(200);
    //Serial.println("Celula 2, OK");
  }

    if (LoadCell_1.getTareTimeoutFlag()) {
    //Serial.println("ERRO Celula 3, favor verificar."); 
    
    while (1);
  }
  else {
    LoadCell_3.setCalFactor(fatorC3); // aplica fato de calibracao
    delay(200);
    //Serial.println("Celula 3, OK");
  }

  //Aciona o contador a cada pulso na descida 

  Serial.println("CLEARDATA");            // Reset da comunicação serial
  Serial.println("LABEL,Date,Time,Acelerador:,Tracao(N):,Torque(N.m):,RPM,Tensao(V):,Corrente(A):,Consumo(W):,Temperatura(C):");   // Nomeia as colunas
} 


void loop() {
  //Termistor
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  
  
  //=============================== SENSOR DE CORRENTE ===============================
  corrente_raw = analogRead(A0);
  C_Voltage = (corrente_raw / 1023.0) * 5000;
  Corrente = ((C_Voltage - ACS_offset) / (mVporAmp*1.15));
  
//=============================== SENSOR DE TENSAO ================================= 
  tensao_raw = analogRead(A5);
  Tensao = (tensao_raw/25.852);
  //Serial.println(Tensao,2);
  
//===================================== RPM =========================================  
if (testState == 2) EIMSK &= ~bit(INT0); // disable INT0 interrupt
  currentMillis = millis();

  if (currentMillis - previousMillis >= printInterval) {
    previousMillis = currentMillis;
    if (testState == 2) {    // testing completed, results are ready
      noInterrupts();
      rpmPeriod = (stopTime - startTime) / rpmMeasurementCycles;
      interrupts();
      rpmFrequency = 1000000.0 / rpmPeriod;
      rpm = rpmFrequency * 60.0/2; // numero de pas
      //Serial.print(rpm);
      //Serial.print(" RPM  ");
      //Serial.print(rpmFrequency);
      //Serial.println(" Hz");
      rpmCount = 0;
      testState = 0;        // clear testState
      EIFR |= bit(INTF0);   // clear INT0 interrupt flag
      EIMSK |= bit(INT0);   // enable INT0 interrupt
    }
  }
 
 //============================== CELULAS DE CARGA =================================
  //DADOS CELULAS
  
  
  LoadCell_1.update();
  LoadCell_2.update();
  LoadCell_3.update();
  
  float Tracao = LoadCell_1.getData();
  float T1 = LoadCell_2.getData();
  float T2 = LoadCell_3.getData();
  

/*
  //print serial celulas de carga
  Serial.print("1: ");
  Serial.println(LoadCell_1.getData());
  Serial.print("2: ");
  Serial.println(LoadCell_2.getData());
  Serial.print("3: ");
  Serial.println(LoadCell_3.getData());
  Serial.println("");
*/
 
  
/* 
  //Serial.print("Celula 1: ");
  Serial.println(Torque,5);
  Serial.print("Celula 2: ");
  Serial.println(T1,5);
  Serial.print("Celula 3: ");
  Serial.println(T2,5);
  //Serial.print(Tensao);
  //Serial.println(" V");
  //Serial.print(corrente));
  //Serial.println(" A"); 
*/  
//=============================== FUNCOES =========================================
Torque = ((-T1+T2)/21.44);
//Serial.println(Torque,3);
//Serial.println(Tracao,1);
//delay(100);
Consumo=Tensao*Corrente;
//=============================== PWM ==============================================
PWM=pulseIn(9,HIGH);
Acelerador = ((PWM-880)/13.14);
//Serial.println(PWM);
//2195
//880




//===================================== EXCEL ============================================================          
   
     if(Acelerador>5){
      Serial.print("DATA,DATE,TIME,");
      Serial.print(Acelerador,0);
      Serial.print(",");
      Serial.print(Tracao,1);
      Serial.print(",");
      Serial.print(Torque,3);
      Serial.print(",");
      Serial.print(rpm);
      Serial.print(",");
      Serial.print(Tensao,2);
      Serial.print(",");
      Serial.print(Corrente,2);
      Serial.print(",");
      Serial.print(T,1);
      Serial.print(",");
      Serial.print(Consumo,1);
      Serial.print(",");
      Serial.println("AUTOSCROLL_40");
      //digitalWrite(LED_ok, HIGH);
      }
      
delay(100);

}

void rpmMeasure() {
  switch (testState) {
    case 0:
      startTime = micros();
      testState = 1;
      break;
    case 1:
      rpmCount++;
      if (rpmCount == rpmMeasurementCycles) {
        stopTime = micros();
        testState = 2;
      }
      break;
  }
}
