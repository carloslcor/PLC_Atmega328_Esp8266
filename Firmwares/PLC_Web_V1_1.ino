// Programa : Modulo I2C Display e Modulo DS1307
// Autor : Arduino e Cia

#include "Wire.h"
#include <LiquidCrystal_I2C.h>

//função que 'quantifica' memoria livre
extern int __bss_end;
extern void *__brkval;

//Para comunicação Serial
String input;
#define MAX_LENGTH 64

// Modulo RTC no endereco 0x68
#define DS1307_ADDRESS 0x68

// Modulo I2C display no endereco 0x27
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);

byte zero = 0x00; 

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  input.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
  lcd.begin (16,4);
  
  //A linha abaixo pode ser retirada apos setar a data e hora
  //SelecionaDataeHora(); 
}

void loop()
{
  SerialInput();
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
  lcd.setCursor(2,1);
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
    lcd.setCursor(6,1);
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

void SelecionaDataeHora()   //Seta a data e a hora do DS1307
{
  byte segundos = 00; //Valores de 0 a 59
  byte minutos = 50; //Valores de 0 a 59
  byte horas = 10; //Valores de 0 a 23
  byte diadasemana = 3; //Valores de 0 a 6 - 0=Domingo, 1 = Segunda, etc.
  byte diadomes = 4; //Valores de 1 a 31
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

//Rotina da Serial
void SerialInput() {
    if (Serial.available()) {
       char c = Serial.read(); // Recebe um caracter
       if(c == '\n') {     // Se foi digitado um ENTER entao processa a String       
         Serial.println("A string digitada foi ");
         Serial.println(input);
         
         input.toUpperCase();    // Converte toda a String para maiusculo
         input.trim();           // Tira os espacos antes e depois
                 
         if(input == "MEMO"){    
          memoriaLivre_print();
         }
         else if (input == "BEMVINDO") {
          Saudacao();
         }       
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

void Saudacao(){
  Serial.println();
  Serial.println("FREEDOM ENGENHARIA");
  Serial.print("Firmware: "); Serial.println("FLS_v1");
  Serial.println("www.freedomengenharia.com.br");
  Serial.println("Digite help para exibir a lista de comandos.");
  Serial.println();
  }
