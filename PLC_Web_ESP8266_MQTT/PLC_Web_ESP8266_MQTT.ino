#include <ESP8266WiFi.h>     
//#include <EEPROMFREEDOM.h>    
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h" 
#include <PubSubClient.h>
WiFiClient wifiClient; 
#include <SoftwareSerial.h>
#include <StringTokenizer.h>                       
    
const char* mqttUser = "yglgzsbj";
const char* mqttPassword = "2jDjjt_zvJL5";
//Para comunicação Serial
String input;
String input2;

int flagAuxSoftSerial = 0;

#define MAX_LENGTH 64
#define MAX_LENGTH2 64
SoftwareSerial mySerial(D3, D4); // RX, TX
String RespARD  = "";
String Arduino[15];

ESP8266WebServer server(80);
//EEPROMFREEDOM eepromfreedom;

//######################## MQTT #############################################

//MQTT Server
const char* BROKER_MQTT = "tailor.cloudmqtt.com"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 13398;                      // Porta do Broker MQTT

#define ID_MQTT  "CAI97"            //Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior. 
#define TOPIC_PUBLISH "BCIBotao1"    //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
#define TOPIC_PUBLISHREACT "@FREEDOM_ENGENHARIA_REACT"    //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
#define TOPIC_SUBSCRIBE "@FREEDOM_ENGENHARIA_WEMOS_ID=TESTE"   //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
PubSubClient MQTT(wifiClient);        // Instancia o Cliente MQTT passando o objeto espClient

//Declaração das Funções
void mantemConexoes();  //Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaMQTT();     //Faz conexão com Broker MQTT
void enviaPacote();     //
void recebePacote(char* topic, byte* payload, unsigned int length);
//#############################################################################

void handleRoot() {
String html ="Tela";

  server.send(200, "text/html", html);
}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
    
void setup() {
  
  Serial.begin(9600);
  input.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
  mySerial.begin(9600);
  input2.reserve(MAX_LENGTH2 + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
  Serial.println("PASSOU DO SETUP!");
  WiFiManager wifiManager;
 
  wifiManager.resetSettings();// define rede e reseta

  wifiManager.autoConnect();
  

  Serial.println("connected...yeey :)");
  
  //server.reset(new ESP8266WebServer(WiFi.localIP(), 80));

  server.on("/", handleRoot);
 
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());
  //MQTT configs tem q ser depois das coisas do Wi-Fi
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  
  MQTT.setCallback(recebePacote); 

  Serial.println("ESP8266 Inicializado!");
 
}

void loop() {

  if (flagAuxSoftSerial == 0){
  server.handleClient();
  mantemConexoes();
  MQTT.loop();
  //SeriaInput();
}
}

void mantemConexoes() {
    if (!MQTT.connected()) {
       conectaMQTT(); 
    }   
}

 void conectaMQTT() { 
    while (!MQTT.connected()) {
        Serial.print("Conectando ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT,mqttUser,mqttPassword)) {
            MQTT.subscribe(TOPIC_SUBSCRIBE);
            Serial.println("Conectado ao Broker com sucesso!");
        } 
        else {
            Serial.println("Noo foi possivel se conectar ao broker.");
            Serial.println("Nova tentatica de conexao em 10s");
            delay(3000);
        }
    }
}

void recebePacote(char* topic, byte* payload, unsigned int length) 
{
    String msg;

    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    Serial.println(msg);
    if(msg == "getAll"){
        //flagAuxSoftSerial = 1;// Para do tratamento no loop
        //Serial.println("Flag Aux Serial: " + flagAuxSoftSerial);
        mySerial.println();
        mySerial.println("STATUS");
        SoftwareSerialInput();
      }
    /*else if(msg == "getAll"){
        mySerial.println("STATUS");
        SoftwareSerialInput();
        MQTT.publish(TOPIC_PUBLISHREACT,input.c_str());
        Serial.print("Mandou pro broken os Valores: ");
        Serial.println(input.c_str());


      }*/
}
void SeriaInput() {
    if (Serial.available()) {
       char c = Serial.read(); // Recebe um caracter
       if(c == '\n') {     // Se foi digitado um ENTER entao processa a String       
         Serial.println("A string digitada foi ");
         Serial.println(input);
         
         input.toUpperCase();    // Converte toda a String para maiusculo
         input.trim();           // Tira os espacos antes e depois
                 
         if(input == "L"){    
          MQTT.publish(TOPIC_PUBLISH, "1");
          Serial.println("Botao1 APERTADO. Payload enviado.");         }
         else if (input == "D") {
          MQTT.publish(TOPIC_PUBLISH, "0");
          Serial.println("Botao1 SOLTO. Payload enviado.");
         } 
         else if (input == "STATUS") {
          mySerial.println("STATUS"); 

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

void SoftwareSerialInput(){
  /*if (Serial.available()) {//Depois devo comentar esse if
    mySerial.write(Serial.read());
  }*/
 if (mySerial.available()) {
  char c;
  while(c != '\n'){
        c = mySerial.read(); // Recebe um caracter
       
       if(c == '\n') {     // Se foi digitado um ENTER entao processa a String       
         //Serial.println("A string Recebida foi ");
         //Serial.println(input);
         RespARD = input;
         StringTokenizer tokens(RespARD,",");
           while(tokens.hasNext()){
           for (int i=0; i <= 15; i++){
            Arduino[i] = tokens.nextToken();       
            }
          /*Serial.println(Arduino[0]);// Status
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
          Serial.println(Arduino[12]);// Atuador 01
          Serial.println(Arduino[13]);// Atuador 02
          Serial.println(Arduino[14]);// Atuador 03
          Serial.println(Arduino[15]);// Atuador 04*/
        }
 

         //Serial.println(RespARD);
         input.toUpperCase();    // Converte toda a String para maiusculo
         input.trim();           // Tira os espacos antes e depois
                 
         if(Arduino[0] == "STATUS"){    
          MQTT.publish(TOPIC_PUBLISHREACT,input.c_str());
          Serial.print("Mandou pro broken os Valores: ");
          Serial.println(input.c_str());
          //flagAuxSoftSerial = 0; // Libera os tratamentos no loop
          //Serial.println("Flag Aux Serial: " + flagAuxSoftSerial);

          //mySerial.println(RespARD);
     
         }
         /*else if (input == "DESLIGA") {
          Serial.println("Desligar LED Pino D13");
          digitalWrite(13,LOW);
         }  */     
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
}
