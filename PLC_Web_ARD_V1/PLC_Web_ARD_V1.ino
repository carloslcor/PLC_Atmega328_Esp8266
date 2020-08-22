// Programa : PLC_WEB_ARD_V1
// Autor : Carlos Rodrigues

#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <avr/wdt.h>
#include "LCD16x4_I2C_Telas.h"
#include "Wire.h"
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
LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);

byte zero = 0x00; 


SoftwareSerial mySerial(50, 51); // RX, TX

//Definição de pinos
const int Press1 = 2;
const int Press2 = 3;
const int DefMot1 = 4;
const int DefMot2 = 5;
const int DefMot3 = 6;
const int DefMot4 = 7;
const int FFase = 8;
const int Motor1 = 9;
const int Motor2 = 10;
const int Motor3 = 11;
const int Motor4 = 12;
const int LedStatus = 13;
const int Buzzer = 14;
const int CisternaBS = 15;
const int CisternaBI = 16;
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

int MotorDaVez = 0;
int MotorExtra = 0;

int Atuador01 = 0;
int Atuador02 = 0;
int Atuador03 = 0;
int Atuador04 = 0;

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
  pinMode(Buzzer, OUTPUT);

  
  Wire.begin();
  mySerial.begin(9600);
  Serial.begin(9600);
  input.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
  input2.reserve(MAX_LENGTH2 + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final

  lcd.begin (16,4);// Inicializa o LCD
  
  //SelecionaDataeHora(); //A linha abaixo pode ser retirada apos setar a data e hora

  LcdTextoLimpar();
  LcdTextoInicializacao();
  delay(10);
  Serial.println("Arduino Inicializado!");
}

void loop()
{
  //Lcd16x4();//lib extra
  wdt_reset();
  SerialInput();
  SoftwareSerialInput();
  //LerEntradas();
  //RevezamentoMotores();
  

}//Final do loop

void SelecionaDataeHora()   //Seta a data e a hora do DS1307
{
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 03; //Valores de 0 a 59
  byte horas = 10; //Valores de 0 a 23
  byte diadasemana = 2; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 31; //Valores de 1 a 31
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
        Serial.print("Enviou "); 
        Serial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","+DefMot1State+","+DefMot2State+","+DefMot3State+","+DefMot4State+","+FFaseState+","+MotorDaVez+","+MotorExtra+","+Atuador01+","+Atuador02+","+Atuador03+","+Atuador04);
        //mySerial.print("String:");
        //mySerial.println(TestString+","+Press1State+","+Press2State+","+CisternaBSState+","+CisternaBIState+","+Mot1State+","+Mot2State+","+Mot3State+","+Mot4State+","+FFaseState+","+MotorDaVez+","+MotorExtra);
        //mySerial.println();
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
  lcd.print("    PLC Web     ");
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
  lcd.print("    PLC Web     ");
  lcd.setCursor(0,2);
  lcd.print("Inicializando OK");
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
}
void RevezamentoMotores(){
   // Press1State = digitalRead(Press1);
    //Press2State = digitalRead(Press2);
    Serial.println("Entrou no Revezamento de motores");
    FFaseState = digitalRead(FFase);
    if(FFaseState == LOW){
      switch (MotorDaVez){
        case 1:
        if(DefMot1State == 0){
        digitalWrite(Motor1,HIGH);
        Serial.println("Motor 1 Ligado");
        lcd.setCursor(0,2);
        lcd.print("Motor1 Ligado   ");
        }
        MotorDaVez = MotorDaVez+1;
        delay(1000);
        break;
        case 2:
        if(DefMot2State == 0){
        digitalWrite(Motor2,HIGH);
        Serial.println("Motor 2 Ligado");
        lcd.setCursor(0,2);
        lcd.print("Motor2 Ligado   ");
        }
        MotorDaVez = MotorDaVez+1;
        delay(1000);
        break;
        case 3:
        if(DefMot3State == 0){
        digitalWrite(Motor3,HIGH);
        Serial.println("Motor 3 Ligado");
        lcd.setCursor(0,2);
        lcd.print("Motor3 Ligado   ");
        }
        MotorDaVez = MotorDaVez+1;
        delay(1000);
        break;
        case 4:
        if(DefMot4State == 0){
        digitalWrite(Motor4,HIGH);
        Serial.println("Motor 4 Ligado");
        lcd.setCursor(0,2);
        lcd.print("Motor4 Ligado   ");
        }
        MotorDaVez = 1;
        delay(1000);
        break;
        }
      }
    else Serial.println("Defeito na Rede");
}
