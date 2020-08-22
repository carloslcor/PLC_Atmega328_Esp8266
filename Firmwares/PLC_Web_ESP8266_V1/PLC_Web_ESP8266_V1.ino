/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
#include <StringTokenizer.h>

//Para comunicação Serial
String input;
#define MAX_LENGTH 64

SoftwareSerial mySerial(D3, D4); // RX, TX

String RespARD  = "";
String Arduino[11];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  input.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final

  Serial.println("ESP8266 Inicializado!");
}

void loop() { // run over and over
  SoftwareSerialInput();
}
void SoftwareSerialInput(){
  if (Serial.available()) {//Depois devo comentar esse if
    mySerial.write(Serial.read());
  }
 if (mySerial.available()) {
       char c = mySerial.read(); // Recebe um caracter
       if(c == '\n') {     // Se foi digitado um ENTER entao processa a String       
         Serial.println("A string Recebida foi ");
         //Serial.println(input);
         RespARD = input;
         StringTokenizer tokens(RespARD,",");
           while(tokens.hasNext()){
           for (int i=0; i <= 11; i++){
            Arduino[i] = tokens.nextToken();       
            }
          Serial.println(Arduino[0]);// Nao Usa
          Serial.println(Arduino[1]);// Pressostato 1
          Serial.println(Arduino[2]);// Pressostato 2
          Serial.println(Arduino[3]);// Cisterta BS
          Serial.println(Arduino[4]);// Cisterta BI
          Serial.println(Arduino[5]);// Motor 01 
          Serial.println(Arduino[6]);// Motor 02
          Serial.println(Arduino[7]);// Motor 03
          Serial.println(Arduino[8]);// Motor 04
          Serial.println(Arduino[9]);// FFaseState
          Serial.println(Arduino[10]);// MotorDaVez
          Serial.println(Arduino[11]);// MotorExtra
        }
 

         //Serial.println(RespARD);
         input.toUpperCase();    // Converte toda a String para maiusculo
         input.trim();           // Tira os espacos antes e depois
                 
         if(input == "STATUS"){    
          Serial.println("Mandou pro Arduino Valores"); 
          mySerial.println(RespARD);
     
         }
         else if (input == "DESLIGA") {
          Serial.println("Desligar LED Pino D13");
          digitalWrite(13,LOW);
         }       
         input = "";// Limpa a String para comecar a armazenar de novo         
       } 
    else {    
         // Se nao veio um ENTER entao vai armazenando até o tamanho maximo que a string pode suportar        
         //if(input.length() < MAX_LENGTH) {
            //if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c = ':') || (c = ',')){ // Aqui estamos filtrando so as letras. Poderia filtrar numeros e espaco por exemplo
                input += c;
            //}
         //}
       }
    }
}
