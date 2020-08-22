#include "Arduino.h"

int aux = 1;

void Lcd16x4(){

  Serial.println(aux);
  delay(100);
  aux++;
  if(aux == 50){
  Serial.println("Chegou em 50! e zerou!");
  aux = 0;
  delay(1000);
  }
  else{}
}

