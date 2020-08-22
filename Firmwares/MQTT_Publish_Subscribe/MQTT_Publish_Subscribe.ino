#include <ESP8266WiFi.h> 
#include <PubSubClient.h>

#define pinBotao1 12  //D6
const char* mqttUser = "yglgzsbj";
const char* mqttPassword = "2jDjjt_zvJL5";

//Para comunicação Serial
String input;
#define MAX_LENGTH 64
//WiFi
const char* SSID = "Samuel-Home";                // SSID / nome da rede WiFi que deseja se conectar
const char* PASSWORD = "20182018";   // Senha da rede WiFi que deseja se conectar
WiFiClient wifiClient;                        
 
//MQTT Server
const char* BROKER_MQTT = "tailor.cloudmqtt.com"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 13398;                      // Porta do Broker MQTT

#define ID_MQTT  "CAI97"            //Informe um ID unico e seu. Caso sejam usados IDs repetidos a ultima conexão irá sobrepor a anterior. 
#define TOPIC_PUBLISH "BCIBotao1"    //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
#define TOPIC_SUBSCRIBE "MACBOOK"   //Informe um Tópico único. Caso sejam usados tópicos em duplicidade, o último irá eliminar o anterior.
PubSubClient MQTT(wifiClient);        // Instancia o Cliente MQTT passando o objeto espClient

//Declaração das Funções
void mantemConexoes();  //Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
void conectaWiFi();     //Faz conexão com WiFi
void conectaMQTT();     //Faz conexão com Broker MQTT
void enviaPacote();     //
void recebePacote(char* topic, byte* payload, unsigned int length);


void setup() {
  pinMode(pinBotao1, INPUT_PULLUP);         

  Serial.begin(9600);
  input.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
  
  conectaWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  
  MQTT.setCallback(recebePacote); 

    Serial.println("ESP8266 Inicializado!");
 
}

void loop() {
  mantemConexoes();
  enviaValores();
  MQTT.loop();
  SeriaInput();
}

void mantemConexoes() {
    if (!MQTT.connected()) {
       conectaMQTT(); 
    }
    
    conectaWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void conectaWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
     return;
  }
        
  Serial.print("Conectando-se na rede: ");
  Serial.print(SSID);
  Serial.println("  Aguarde!");

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI  
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(SSID);  
  Serial.print("  IP obtido: ");
  Serial.println(WiFi.localIP()); 
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

void enviaValores() {
static bool estadoBotao1 = HIGH;
static bool estadoBotao1Ant = HIGH;
static unsigned long debounceBotao1;

  estadoBotao1 = digitalRead(pinBotao1);
  if (  (millis() - debounceBotao1) > 30 ) {  //Elimina efeito Bouncing
     if (!estadoBotao1 && estadoBotao1Ant) {

        //Botao Apertado     
        MQTT.publish(TOPIC_PUBLISH, "1");
        Serial.println("Botao1 APERTADO. Payload enviado.");
        
        debounceBotao1 = millis();
     } else if (estadoBotao1 && !estadoBotao1Ant) {

        //Botao Solto
        MQTT.publish(TOPIC_PUBLISH, "0");
        Serial.println("Botao1 SOLTO. Payload enviado.");
        
        debounceBotao1 = millis();
     }
     
  }
  estadoBotao1Ant = estadoBotao1;
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
    /*
    if (msg == "0") {
       digitalWrite(pinLED1, LOW);
    }

    if (msg == "1") {
       digitalWrite(pinLED1, HIGH);
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
