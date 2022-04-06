#include "LedControl.h"
#include <Wire.h>
#include <INA219_WE.h>

#define inaadress0 0x40
#define inaadress1 0x41



INA219_WE inam0 = INA219_WE(inaadress0);
INA219_WE inam1 = INA219_WE(inaadress1);
LedControl lc=LedControl(4,3,2,2);
byte stopp[8]={B00111100,
B01000010,
B10011001,
B10111101,
B10111101,
B10011001,
B01000010,
B00111100};

byte aufgeht[8]={B00011000,
B00100100,
B01011010,
B10100101,
B01011010,
B10100101,
B01000010,
B10000001};

byte zugeht[8]={B10000001,
B01000010,
B10100101,
B01011010,
B10100101,
B01011010,
B00100100,
B00011000};


byte notaus[8]={B11000011,
B11100111,
B01111110,
B00111100,
B00111100,
B01111110,
B11100111,
B11000011};

byte block[8]={B11111111,
B11111111,
B11000011,
B11000011,
B11000011,
B11000011,
B11111111,
B11111111};

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 9); // CE, CSN

const byte address[6] = "00001";
const int R1 = A0 ;          //M1 K1
const int R2 = A1 ;          //M1 K2
const int R3 = A2 ;          //M2 K1
const int R4 = A3 ;          //M2 K2

const int T1 = 6 ;         //Taster AUF
//const int T2 = 6 ;         //Taster ZU
const int T3 = 7 ;         //STOP

const float maxstrom = 3000;   //Strombegrenzung
boolean T1status = false;
boolean T2status = false;
boolean T3status = false;

long previousMillis = 0;
boolean auf = false;              //Tor in AUF-Bewegung
boolean zu = false;               //Tor in ZU-Bewegung
byte modus;
int zeitaufli = 11000;                 //Zeit zum öffnen
int zeitzuli = 11000;                  //Zeit zum schliessen
int zeitaufre = 12000;                 //Zeit zum öffnen
int zeitzure = 12000;                  //Zeit zum schliessen
int zeitwarten = 2000;              //Zeit Ampelmatrix nachleuchten
int zeitversatz = 2000;             //Zeit Veratz der Motoren beim anlauf
void setup() {
 Serial.begin(9600);
  Wire.begin();
  if(!inam0.init()){
    Serial.println("INA Sensor M1 verbunden");
  }
  if(!inam1.init()){
    Serial.println("INA Sensor M2 verbunden");
  }
  


  
 lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  lc.shutdown(1,false);
  lc.setIntensity(1,8);
  lc.clearDisplay(1);
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);

  pinMode(T1, INPUT);
 // pinMode(T2, INPUT_PULLUP);
  pinMode(T3, INPUT);
  
  
  //pinMode(strom1, INPUT);
  //pinMode(strom2, INPUT);
  //pinMode(strom3, INPUT);
  //pinMode(strom4, INPUT);
  
  //Alle Relais lösen wegen Umkehrung
  digitalWrite(R1, HIGH);
  digitalWrite(R2, HIGH);
  digitalWrite(R3, HIGH);
  digitalWrite(R4, HIGH);
  
}

void loop() {
float current_mA0 = 0.0;
float current_mA1 = 0.0;

  delay(20);
unsigned long currentMillis = millis();  
  
switch(modus) {  
  
 case 0:

    if(digitalRead(T1) == HIGH && auf == false)
   modus=1;
    if(digitalRead(T1) == HIGH && auf == true)
    modus=5;
 //   if(digitalRead(T2) == LOW && zu == false)
 //   modus=4;
    break;
  
 case 1:                                  // Motor 1 Start
  for (int i=0; i<8; i++){
    lc.setRow(0,i,aufgeht[i]);
    lc.setRow(1,i,aufgeht[i]);
  }
    {
      digitalWrite(R1, LOW);
current_mA0 = inam0.getCurrent_mA();
current_mA1 = inam1.getCurrent_mA();
Serial.print("Current[mA] M1: "); Serial.print(current_mA0);
Serial.print(" Current[mA] 21: "); Serial.println(current_mA1);
        previousMillis = millis();
    auf = true;
    modus++;
    }
    break;
    
 case 2:                                                // Motor 2 Start nach Zeitversatz
 Serial.println("Case2");

    if((millis() - previousMillis >= zeitversatz) && (auf == true)) {
    digitalWrite(R3, LOW);
delay(500);    
    modus++;
    }
    break;
    
 case 3:                                                // beide Motoren laufen bis zeitaufre abgelaufen ist
 Serial.print("Tor auf      ");
current_mA0 = inam0.getCurrent_mA();
current_mA1 = inam1.getCurrent_mA();
Serial.print("Current[mA] M1: "); Serial.print(current_mA0);
Serial.print(" Current[mA] 21: "); Serial.print(current_mA1);
Serial.print(" Millis "); Serial.println(millis() - previousMillis);
 
    if(digitalRead(T3) == HIGH || millis() - previousMillis >= zeitaufre || current_mA0 > maxstrom || current_mA1 > maxstrom){
      for (int i=0; i<8; i++){
    lc.setRow(0,i,stopp[i]);
     lc.setRow(1,i,stopp[i]);
  }
   
    
   if(current_mA0 > maxstrom){
    digitalWrite(R1, HIGH);
       for (int i=0; i<8; i++){
    lc.setRow(0,i,block[i]);
     lc.setRow(1,i,stopp[i]);
  }
      modus = 4;
   }
   if(current_mA1 > maxstrom){
      zu = false;
    digitalWrite(R1, HIGH);
    digitalWrite(R3, HIGH);
      modus = 0;
      break;
   }
   
      
   
    digitalWrite(R1, HIGH);
    zu = false;
    modus = 4;
    }
    
    break;


     case 4:                                  // beide Motoren laufen bis zeitaufre abgelaufen ist
 Serial.print("linker Fluegel oeffnen      ");
current_mA0 = inam0.getCurrent_mA();
current_mA1 = inam1.getCurrent_mA();
Serial.print("Current[mA] M1: "); Serial.print(current_mA0);
Serial.print(" Current[mA] 21: "); Serial.print(current_mA1);
Serial.print(" Millis "); Serial.println(millis() - previousMillis);
 
    if(digitalRead(T3) == HIGH || millis() - previousMillis >= zeitaufli || current_mA0 > maxstrom || current_mA1 > maxstrom){
      for (int i=0; i<8; i++){
    lc.setRow(0,i,stopp[i]);
     lc.setRow(1,i,stopp[i]);
  }
   
    digitalWrite(R3, HIGH);
    digitalWrite(R1, HIGH);
   
    
    zu = false;
    modus = 9;
    }
    
    break;
    
 case 5: 
 for (int i=0; i<8; i++){
    lc.setRow(0,i,zugeht[i]);
    lc.setRow(1,i,zugeht[i]);
  }
    {digitalWrite(R2, LOW);
     
    previousMillis = millis();
    zu = true;
    modus++;
    }
    break;   
    
 case 6:
 Serial.println("Case5");
    if((millis() - previousMillis >= zeitversatz) && (zu == true)) {
      
    digitalWrite(R4, LOW);
  
    modus++;
    }
delay(500);
    break;   
    
 case 7:
 Serial.print("Tor zu      ");
current_mA0 = inam0.getCurrent_mA();
current_mA1 = inam1.getCurrent_mA();
Serial.print("Current[mA] M1: "); Serial.print(current_mA0);
Serial.print(" Current[mA] M2: "); Serial.print(current_mA1);
Serial.print(" Millis "); Serial.println(millis() - previousMillis);
 
    if(digitalRead(T1) == HIGH || millis() - previousMillis >= zeitzure || current_mA0 > maxstrom || current_mA1 > maxstrom){
      for (int i=0; i<8; i++){
    lc.setRow(0,i,stopp[i]);
     lc.setRow(1,i,stopp[i]);
  }
   
    
   if(current_mA0 > maxstrom){
    digitalWrite(R2, HIGH);
       for (int i=0; i<8; i++){
    lc.setRow(0,i,block[i]);
     lc.setRow(1,i,stopp[i]);
  }
      modus = 8;
   }
   if(current_mA1 > maxstrom){
      auf = false;
    digitalWrite(R2, HIGH);
    digitalWrite(R4, HIGH);
      modus = 0;
      break;
   }
   
      
   
    
    auf = false;
    modus = 8;
    }
    
    break;  

case 8:
Serial.print("linker Fluegel schliessen      ");
current_mA0 = inam0.getCurrent_mA();
current_mA1 = inam1.getCurrent_mA();
Serial.print("Current[mA] M1: "); Serial.print(current_mA0);
Serial.print(" Current[mA] 21: "); Serial.print(current_mA1);
Serial.print(" Millis "); Serial.println(millis() - previousMillis);
 
    if(digitalRead(T3) == HIGH || millis() - previousMillis >= zeitzuli + zeitversatz || current_mA1 > maxstrom){
      for (int i=0; i<8; i++){
    lc.setRow(0,i,stopp[i]);
     lc.setRow(1,i,stopp[i]);
  }
   
    digitalWrite(R2, HIGH);
    digitalWrite(R4, HIGH);
   
    
    zu = false;
    modus = 9;
    }
    
    break;    

case 9:
 previousMillis = millis();
 modus = 10;
    break; 
    
    
    
    case 10:
    
 
    if(digitalRead(T3) == HIGH || millis() - previousMillis >= zeitwarten){
      lc.clearDisplay(0);
      lc.clearDisplay(1);
    modus = 0;
    
    break;   



    }
}

    //EMEREGENCY-STOP
    if(digitalRead(T3) == HIGH) {
      for (int i=0; i<8; i++){
    lc.setRow(0,i,notaus[i]);
    lc.setRow(1,i,notaus[i]);
  }
    digitalWrite(R1, HIGH);
    digitalWrite(R2, HIGH);
    digitalWrite(R3, HIGH);
    digitalWrite(R4, HIGH);
    
    auf = false;
    zu = false;
    modus = 0;
    }

    
   
}