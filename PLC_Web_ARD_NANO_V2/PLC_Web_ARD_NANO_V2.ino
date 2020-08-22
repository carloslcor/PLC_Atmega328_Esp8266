// Programa : PLC_WEB_ARD_V1
// Autor : Carlos Rodrigues

#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <avr/wdt.h>
#include "LCD16x4_I2C_Telas.h"
#include "Wire.h"
#include "TimerOne.h"

//função que 'quantifica' memoria livre
extern int __bss_end;
extern void *__brkval;

//Para comunicação Serial
String input;
#define MAX_LENGTH 64

//Para comunicação SofwareSerial
String input2;
#define MAX_LENGTH2 64

String TestString = "STATUS";

// Modulo RTC no endereco 0x68
#define DS1307_ADDRESS 0x68

// Modulo I2C display no endereco 0x27
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);

byte zero = 0x00; 


SoftwareSerial mySerial(5, 6); // RX, TX

//Definição de pinos
const int FFase = 2;
const int CisternaBS = 3;
const int CisternaBI = 4;
const int Motor2 = 7; //R2 da Placa
const int Motor1 = 8; //R1 da Placa
const int Motor3 = 9;//R3 da Placa
const int Motor4 = 10;//R4 da Placa
const int Press1 = 11;
const int Press2 = 12;
const int LedStatus = 13;
const int DefMot1 = 14;//A0
const int DefMot2 = 15;//A1
const int DefMot3 = 16;//A2  
const int DefMot4 = 17;//A3

//Auxiliares defeito dos motores
int auxdefmot1 = 0;
int auxdefmot2 = 0;
int auxdefmot3 = 0;
int auxdefmot4 = 0;

// Variaveis auxiliares dos pinos
int Press1State = 0;  
int Press2State = 0;   
int CisternaBSState = 0; 
int CisternaBIState = 0;         
int DefMot1State = 0;   
int DefMot2State = 0;     
int DefMot3State = 0;     
int DefMot4State = 0;     
int FFaseState = 0;    
//Secundarias
int MotorDaVez = 1;
int MotorExtra = 0;
int Press1OK = 0;
int Press2OK = 0;

//Não utilizadas ainda
int Atuador01 = 0;
int Atuador02 = 0;
int Atuador03 = 0;
int Atuador04 = 0;

//Aux Contadores
int Motor1Atuando = 0;
int Motor2Atuando = 0;
int Motor3Atuando = 0;
int Motor4Atuando = 0;

//Contadores
long int Motor1Tempo = 0;
long int Motor2Tempo = 0;
long int Motor3Tempo = 0;
long int Motor4Tempo = 0;

//Conta tempo
int segundos = 0;
int minutos = 0;
int horas = 0;

void setup()
{
  wdt_enable(WDTO_8S);//WDTO_8S, WDTO_4S, WDTO_1S, WDTO_500MS, WDTO_250MS,WDTO_120MS, WDTO_60MS, WDTO_30MS e WDTO_15MS
  //Inputs
  pinMode(Press1, INPUT_PULLUP);
  pinMode(Press2, INPUT_PULLUP);
  pinMode(CisternaBS, INPUT_PULLUP);
  pinMode(CisternaBI, INPUT_PULLUP);
  pinMode(DefMot1, INPUT_PULLUP);
  pinMode(DefMot2, INPUT_PULLUP);
  pinMode(DefMot3, INPUT_PULLUP);
  pinMode(DefMot4, INPUT_PULLUP);
  pinMode(FFase, INPUT_PULLUP);
  //Outputs
  pinMode(Motor1, OUTPUT);
  pinMode(Motor2, OUTPUT);
  pinMode(Motor3, OUTPUT);
  pinMode(Motor4, OUTPUT);
  pinMode(LedStatus, OUTPUT);

  Timer1.initialize(1000000); // Inicializa o Timer1 e configura para um período de 1 segundos
  Timer1.attachInterrupt(callback); // Configura a função callback() como a função para ser chamada a cada interrupção do Timer1            


  Wire.begin();
  mySerial.begin(9600);
  Serial.begin(9600);
  input.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
  input2.reserve(MAX_LENGTH2 + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final

  lcd.begin (16,4);// Inicializa o LCD
  
  //SelecionaDataeHora(); //A linha abaixo pode ser retirada apos setar a data e hora

  LcdTextoLimpar();
  LcdTextoInicializacao();
  EnviaStatus();
  Serial.println("Arduino Nano Inicializado!");
}

void loop()
{
  //Lcd16x4();//lib extra
  wdt_reset();
  SerialInput();
  SoftwareSerialInput();
  LerEntradas();
}//Final do loop

void SelecionaDataeHora()   //Seta a data e a hora do DS1307
{
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 56; //Valores de 0 a 59
  byte horas = 16; //Valores de 0 a 23
  byte diadasemana = 3; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 25; //Valores de 1 a 31
  byte mes = 3; //Valores de 1 a 12
  byte ano = 20; //Valores de 0 a 99
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //Stop no CI para que o mesmo possa receber os dados

  //As linhas abaixo escrevem no CI os valores de 
  //data e hora que foram colocados nas variaveis acima
  Wire.write(ConverteParaBCD(segundos));
  Wire.write(ConverteParaBCD(minutos));
  Wire.write(ConverteParaBCD(horas));
  Wire.write(ConverteParaBCD(diadasemana));
  Wire.write(ConverteParaBCD(diadomes));
  Wire.write(ConverteParaBCD(mes));
  Wire.write(ConverteParaBCD(ano));
  Wire.write(zero);
  Wire.endTransmission(); 
}

byte ConverteParaBCD(byte val)
{ 
  //Converte o número de decimal para BCD
  return ( (val/10*16) + (val%10) );
}

byte ConverteparaDecimal(byte val)  
{ 
  //Converte de BCD para decimal
  return ( (val/16*10) + (val%16) );
}

//Rotina da SoftwareSerial 
void SoftwareSerialInput() {
    if (mySerial.available()) {
       char c = mySerial.read(); // Recebe um caracter
       if(c == '\n') {     // Se foi digitado um ENTER entao processa a String       
         Serial.println("Recebido pela Software Serial: ");
         Serial.println(input2);
         
         input2.toUpperCase();    // Converte toda a String para maiusculo
         input2.trim();           // Tira os espacos antes e depois
                 
         if(input2 == "MEMO"){    
          memoriaLivre_print();
         }
         else if (input2 == "BEMVINDO") {
          Saudacao();
         }    
         else if (input2 == "HORA") {
           LcdTextoLimpar();
           LcdTextoRTC();
         }      
         else if (input2 == "INICIAR") {
           LcdTextoLimpar();
           LcdTextoInicializacao();
         } 
        else if (input2 == "STATUS") {
        Serial.println("3-Recebeu STATUS do ESP8266"); 
        //Serial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","+DefMot1State+","+DefMot2State+","+DefMot3State+","+DefMot4State+","+FFaseState+","+MotorDaVez+","+MotorExtra+","+Atuador01+","+Atuador02+","+Atuador03+","+Atuador04);
        //mySerial.print("String:");
        //mySerial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","+Mot1State+","+Mot2State+","+Mot3State+","+Mot4State+","+FFaseState+","+MotorDaVez+","+MotorExtra);
        //mySerial.println();
        //mySerial.println();
        Serial.println("4-Enviou os valores para o ESP8266:"); 
        Serial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","+DefMot1State+","+DefMot2State+","+DefMot3State+","+DefMot4State+","+FFaseState+","+MotorDaVez+","+MotorExtra+","+Atuador01+","+Atuador02+","+Atuador03+","+Atuador04);
        
        mySerial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","+DefMot1State+","+DefMot2State+","+DefMot3State+","+DefMot4State+","+FFaseState+","+MotorDaVez+","+MotorExtra+","+Atuador01+","+Atuador02+","+Atuador03+","+Atuador04);
     }     
         input2 = "";// Limpa a String para comecar a armazenar de novo         
       } 
    else {    
         // Se nao veio um ENTER entao vai armazenando até o tamanho maximo que a string pode suportar        
         if(input2.length() < MAX_LENGTH2) {
            if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')){ // Aqui estamos filtrando so as letras. Poderia filtrar numeros e espaco por exemplo
                input2 += c;
            }
         }
       }
    }
}
////////////////////////////////
//Rotina da Serial
void SerialInput() {
    if (Serial.available()) {
       char c = Serial.read(); // Recebe um caracter
       if(c == '\n') {     // Se foi digitado um ENTER entao processa a String       
         Serial.println("Recebido pela Serial Nativa: ");
         Serial.println(input);
         
         input.toUpperCase();    // Converte toda a String para maiusculo
         input.trim();           // Tira os espacos antes e depois
                 
         if(input == "MEMO"){    
          memoriaLivre_print();
         }
         else if (input == "BEMVINDO") {
          Saudacao();
         }    
         else if (input == "HORA") {
           LcdTextoLimpar();
           LcdTextoRTC();
         }      
         else if (input == "INICIAR") {
           LcdTextoLimpar();
           LcdTextoInicializacao();
         }  
           else if (input == "TEMPO") {
           Serial.print("Motor 01 "); 
           Serial.println(Motor1Tempo);
           Serial.print("Motor 02 "); 
           Serial.println(Motor2Tempo);
           Serial.print("Motor 03 "); 
           Serial.println(Motor3Tempo);
           Serial.print("Motor 04 "); 
           Serial.println(Motor4Tempo);
         }  

         /*else if (input == "STATUS") {
             mySerial.write("STATUS");
             Serial.println("Mandei Status pro esp"); 
         } */    
         input = "";// Limpa a String para comecar a armazenar de novo         
       } 
    else {    
         // Se nao veio um ENTER entao vai armazenando até o tamanho maximo que a string pode suportar        
         if(input.length() < MAX_LENGTH) {
            if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')){ // Aqui estamos filtrando so as letras. Poderia filtrar numeros e espaco por exemplo
                input += c;
            }
         }
       }
    }
}


//Exibe memoria livre
int memoriaLivre(){
  int memLivre;
  if((int)__brkval == 0)
    memLivre = ((int)&memLivre) - ((int)&__bss_end);
  else
    memLivre = ((int)&memLivre) - ((int)&__brkval);
  return memLivre;
  }
  
void memoriaLivre_print(){
  Serial.print("Memória Livre: "); Serial.print(memoriaLivre(),DEC); Serial.println("kb");
  }
///////////////////////////////////////////////////////////////////////////////////////////////
// Mensagens via Serial
void Saudacao(){
  Serial.println();
  Serial.println("FREEDOM ENGENHARIA");
  Serial.print("Firmware: "); Serial.println("FLS_v1");
  Serial.println("www.freedomengenharia.com.br");
  Serial.println("Digite help para exibir a lista de comandos.");
  Serial.println();
  }
 ///////////////////////////////////////////////////////////////////////////////////////////////   
  //Telas do LCD
  void LcdTextoRTC(){
  // Le os valores (data e hora) do modulo DS1307
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_ADDRESS, 7);
  int segundos = ConverteparaDecimal(Wire.read());
  int minutos = ConverteparaDecimal(Wire.read());
  int horas = ConverteparaDecimal(Wire.read() & 0b111111);
  int diadasemana = ConverteparaDecimal(Wire.read()); 
  int diadomes = ConverteparaDecimal(Wire.read());
  int mes = ConverteparaDecimal(Wire.read());
  int ano = ConverteparaDecimal(Wire.read());

  // Mostra os dados no display
  lcd.setCursor(0,0);
  lcd.print("Gurgel Geradores");
  lcd.setCursor(0,2);
  lcd.print("     ");
  // Acrescenta o 0 (zero) se a hora for menor do que 10
  if (horas <10)
    lcd.print("0");
  lcd.print(horas);
  lcd.print(":");
  // Acrescenta o 0 (zero) se minutos for menor do que 10
  if (minutos < 10)
     lcd.print("0");
  lcd.print(minutos);
  lcd.setCursor(2,3);
  // Mostra o dia da semana
  switch(diadasemana)
    {
      case 0:lcd.print("Dom");
      break;
      case 1:lcd.print("Seg");
      break;
      case 2:lcd.print("Ter");
      break;
      case 3:lcd.print("Quar");
      break;
      case 4:lcd.print("Qui");
      break;
      case 5:lcd.print("Sex");
      break;
      case 6:lcd.print("Sab");
    }
    lcd.setCursor(6,3);
    // Acrescenta o 0 (zero) se dia do mes for menor do que 10
    if (diadomes < 10)
      lcd.print("0");
    lcd.print(diadomes);
    lcd.print("/");
    // Acrescenta o 0 (zero) se mes for menor do que 10
    if (mes < 10)
      lcd.print("0");
    lcd.print(mes);
    lcd.print("/");
    lcd.print(ano);  
}

  void LcdTextoInicializacao(){ 
  lcd.setCursor(0,0);
  lcd.print("Gurgel Geradores");
  lcd.setCursor(0,2);
  lcd.print(" Incializando   ");
  lcd.setCursor(0,3);
  lcd.print(" o Sistema.     ");
  //delay(350);
  lcd.setCursor(0,3);
  lcd.print(" o Sistema..    ");
  //delay(350);
  lcd.setCursor(0,3);
  lcd.print(" o Sistema...   ");
  //delay(350);
  lcd.setCursor(0,3);
  lcd.print(" o Sistema....  ");
  //delay(350);
  lcd.setCursor(0,3);
  lcd.print(" o Sistema..... ");
  //delay(350);
  LcdTextoLimpar();
  //lcd.setCursor(0,0);
  //lcd.print("Gurgel Geradores");

}
  void LcdTextoLimpar(){
  lcd.setCursor(0,0);
  lcd.print("                ");//PLC WEB
  lcd.setCursor(0,1);
  lcd.print("                ");//Bomba em funcionamento
  lcd.setCursor(0,2);
  lcd.print("                ");//Avisa de defeito na bomba e 
  lcd.setCursor(0,3);
  lcd.print("                ");//
} 
///////////////////////////////////////////////////////////////////////////////////////////////

//Revezamento dos motores
void LerEntradas(){
    FFaseState = digitalRead(FFase);
    Press1State = digitalRead(Press1);
    Press2State = digitalRead(Press2);
    CisternaBSState = digitalRead(CisternaBS);
    CisternaBIState = digitalRead(CisternaBI); 
    DefMot1State = digitalRead(DefMot1);
    DefMot2State = digitalRead(DefMot2);
    DefMot3State = digitalRead(DefMot3);
    DefMot4State = digitalRead(DefMot4); 

 
  //Serial.print("DefMot1State: ");
   //Serial.println(DefMot1State);
    if(DefMot1State == 1){
      lcd.setCursor(0,1); lcd.print("MT1 OK ");
      //auxdefmot1 = 0;
      //Serial.println("Motor 1 OK");

  }
    else if(DefMot1State == 0){
      lcd.setCursor(0,1); lcd.print("MT1 DEF");
      //digitalWrite(Motor1,LOW);
      //auxdefmot1 = 1;
      //Serial.println("Motor 1 DEF");
      //delay(1000);
  }
    if(DefMot2State == 1){
      lcd.setCursor(8,1); lcd.print("MT2 OK ");
  }
    else if(DefMot2State == 0){
      lcd.setCursor(8,1); lcd.print("MT2 DEF");
  }  
    if(DefMot3State == 1){
      lcd.setCursor(0,2); lcd.print("MT3 OK ");
  }
    else if(DefMot3State == 0){
      lcd.setCursor(0,2); lcd.print("MT3 DEF");
  }  
    if(DefMot4State == 1){
      lcd.setCursor(8,2); lcd.print("MT4 OK ");
  }
    else if(DefMot4State == 0){
      lcd.setCursor(8,2); lcd.print("MT4 DEF");
  }  
    if(FFaseState == 1){
      lcd.setCursor(0,3); lcd.print("RED OK ");
  }
    else if(FFaseState == 0){
      lcd.setCursor(0,3); lcd.print("RED DEF");
  }  
    if((CisternaBSState == 1) and (CisternaBIState == 1)){
      lcd.setCursor(8,3); lcd.print("CIS 100%");
  }
    else if((CisternaBSState == 0) and (CisternaBIState == 1)){
      lcd.setCursor(8,3); lcd.print("CIS 50% ");
  }    
    else if((CisternaBSState == 0) and (CisternaBIState == 0)){
      lcd.setCursor(8,3); lcd.print("CIS 0%  ");    
  }
    if((FFaseState == 1) and (CisternaBIState == 1) and ((DefMot1State == 1) or (DefMot2State == 1) or (DefMot3State == 1) or (DefMot4State == 1))){
      lcd.setCursor(0,0); lcd.print("Gurgel");     
      //Serial.println("Sistema Operante");
  }
    else {
      //Serial.println("Sistema Inoperante");
      lcd.setCursor(0,0); lcd.print("Defeit"); 
  } 

//Acinamento dos motores
    if ((Press1State == 1) and (Press2State == 1)){
      //Serial.println("Desliga todos os Motores");
      lcd.setCursor(7,0); lcd.print("P1/P2:OFF"); 
    }
    else if ((Press1State == 0) and (Press2State == 1)){
      //Serial.println("Liga um Motor");
      lcd.setCursor(7,0); lcd.print("P1:ON    ");   
    }
    else if ((Press1State == 0) and (Press2State == 0)){
      //Serial.println("Liga dois motores");
      lcd.setCursor(7,0); lcd.print("P1/P2:ON ");  
    }

    RevezamentoMotores();
}
void RevezamentoMotores(){
    //Press1State = digitalRead(Press1);
    //Press2State = digitalRead(Press2);
    //Serial.print("Press1: "); Serial.println(Press1State);
    //Serial.print("Press2: "); Serial.println(Press2State);
    //Serial.print("Motor da vez: "); Serial.println(MotorDaVez);
    if(FFaseState == HIGH){
      if ((Press1State == LOW) and (Press1OK == 0)){
      switch (MotorDaVez){
        case 1:
        if(DefMot1State == HIGH){
        digitalWrite(Motor1,HIGH);
        Serial.println("Motor 1 Ligado");
        MotorDaVez = MotorDaVez+1;
        Motor1Atuando = 1;
        Press1OK = 1;
        }
        else if(DefMot1State == LOW){
        MotorDaVez = MotorDaVez+1;
        Motor1Atuando = 0;
        Serial.println("Motor 1 Defeito");
        }
        break;
      
        case 2:
        if(DefMot2State == HIGH){
        digitalWrite(Motor2,HIGH);
        Serial.println("Motor 2 Ligado");
        MotorDaVez = MotorDaVez+1;
        Motor2Atuando = 1;
        Press1OK = 1;
        }
        else if(DefMot2State == LOW){
        MotorDaVez = MotorDaVez+1;
        Motor2Atuando = 0;
        Serial.println("Motor 2 Defeito");
        }
        break;

        case 3:
        if(DefMot3State == HIGH){
        digitalWrite(Motor3,HIGH);
        Serial.println("Motor 3 Ligado");
        //lcd.setCursor(0,2);
        //lcd.print("Motor3 Ligado   ");
        MotorDaVez = MotorDaVez+1;
        Motor3Atuando = 1;
        Press1OK = 1;
        }
        else if(DefMot3State == LOW){
        MotorDaVez = MotorDaVez+1;
        Motor3Atuando = 0;
        //lcd.setCursor(0,1);
        //lcd.print("Motor 03 Defeito");
        Serial.println("Motor 3 Defeito");
        }
        break;

        case 4:
        if(DefMot4State == HIGH){
        digitalWrite(Motor4,HIGH);
        Serial.println("Motor 4 Ligado");
        //lcd.setCursor(0,2);
        //lcd.print("Motor4 Ligado   ");
        MotorDaVez = 1;
        Motor4Atuando = 1;
        Press1OK = 1;
        }
        else if(DefMot4State == LOW){
        MotorDaVez = 1;
        Motor4Atuando = 0;
        //lcd.setCursor(0,1);
        //lcd.print("Motor 04 Defeito");
        Serial.println("Motor 4 Defeito");
        }
        break;
        }
      }
      if ((Press2State == LOW) and (Press2OK == 0)){
         switch (MotorDaVez){
        case 1:
        if(DefMot1State == HIGH){
        digitalWrite(Motor1,HIGH);
        Serial.println("Motor 1 Ligado");
        MotorDaVez = MotorDaVez+1;
        Press2OK = 1;
        }
        else if(DefMot1State == LOW){
        MotorDaVez = MotorDaVez+1;
        Serial.println("Motor 1 Defeito");
        }
        break;
        case 2:
        if(DefMot2State == HIGH){
        digitalWrite(Motor2,HIGH);
        Serial.println("Motor2 Ligado");
        MotorDaVez = MotorDaVez+1;
        Press2OK = 1;
        }
        else if(DefMot2State == LOW){
        MotorDaVez = MotorDaVez+1;
        Serial.println("Motor 2 Defeito");
        }
        break;

        case 3:
        if(DefMot3State == HIGH){
        digitalWrite(Motor3,HIGH);
        Serial.println("Motor 3 Ligado");
        MotorDaVez = MotorDaVez+1;
        Press2OK = 1;        
        }
        else if(DefMot3State == LOW){
        MotorDaVez = MotorDaVez+1;
        Serial.println("Motor 3 Defeito");
        }
        break;

        case 4:
        if(DefMot4State == HIGH){
        digitalWrite(Motor4,HIGH);
        MotorDaVez = 1;
        Press2OK = 1;
        Serial.println("Motor 4 Ligado");
        }
        else if(DefMot1State == LOW){
        MotorDaVez = 1;
        Serial.println("Motor 4 Defeito");
        }
        break;
        }

      }
       if ((Press1State == HIGH) and (Press2State == HIGH)){
        digitalWrite(Motor1,LOW);
        digitalWrite(Motor2,LOW);
        digitalWrite(Motor3,LOW);
        digitalWrite(Motor4,LOW);
        Motor1Atuando = 0;
        Motor2Atuando = 0;
        Motor3Atuando = 0;
        Motor4Atuando = 0;
        Press1OK = 0;
        Press2OK = 0;
       }       
    }
    if(FFaseState == LOW){
        digitalWrite(Motor1,LOW);
        digitalWrite(Motor2,LOW);
        digitalWrite(Motor3,LOW);
        digitalWrite(Motor4,LOW);
        Press1OK = 0;
    }



}

void EnviaStatus(){
  mySerial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","
    +DefMot1State+","+DefMot2State+","+DefMot3State+","+DefMot4State+","+FFaseState+","+MotorDaVez+","
    +MotorExtra+","+Atuador01+","+Atuador02+","+Atuador03+","+Atuador04);
}

void callback(){
segundos ++;
  if(segundos == 2){
    Serial.println("Intervelado de Verificação");
    //Serial.print(Motor1Atuando);
    //Serial.print(Motor2Atuando);
    //Serial.print(Motor3Atuando);
    //Serial.println(Motor4Atuando);
    segundos = 0;

  }
  if((Motor1Atuando == 1) && (DefMot1State == LOW)){
    Press1OK = 0;
    Motor1Atuando = 0;
    delay(1000);
    Serial.println("Deu defeito no motor 1");
  }
  if((Motor2Atuando == 1) && (DefMot2State == LOW)){
    Press1OK = 0;
    Motor2Atuando = 0;
    delay(1000);
    Serial.println("Deu defeito no motor 2");
  }
  if((Motor3Atuando == 1) && (DefMot3State == LOW)){
    Press1OK = 0;
    Motor3Atuando = 0;
    delay(1000);
    Serial.println("Deu defeito no motor 3");
  }
  if((Motor4Atuando == 1) && (DefMot4State == LOW)){
    Press1OK = 0;
    Motor4Atuando = 0;
    delay(1000);
    Serial.println("Deu defeito no motor 4");
  }
/*Serial.print("Horas:");Serial.print(horas);
Serial.print(" Minutos:");Serial.print(minutos);
Serial.print(" Segundos:");Serial.println(segundos);
Serial.println(segundos);

  if(segundos == 60){
    segundos = 0;
    minutos++;
  }
  if(minutos == 60){
    minutos = 0;
    horas++;
  }
  if(horas == 24){
    horas = 0;
  }*/
  
}

