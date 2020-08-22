#include <HTTPFREEDOM.h>
#include "WiFiManager.h"  

HTTPFREEDOM httpFreedom;
WiFiManager wifiManager;

//################### Variaveis referentes aos pinos do Arduino ################### 
int NIVEL = 321;
int TEMPERATURA = 33;
String VALOR_RECEBIDO_TELA = "";
//#################################################################################


String responseJson() {   
  String response = "{\"data\":[";
  response += "{\"nivel\":\"";
  response += NIVEL;
  response += "\"},";
  response += "{\"temperatura\":\"";
  response += TEMPERATURA;
  response += "\"}";
  response += "]}";
  return response;
}

void setup() {

  Serial.begin(115200);
  
  //wifiManager.resetSettings();//reseta a eeprom para voltar pro modo AP, deixe comentando.

  wifiManager.autoConnect();

  httpFreedom.defaultSetting(responseJson(), "<!DOCTYPE html> <html> <head> <title>Tela de Estudo</title> <meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, minimumscale=1.0, maximum-scale=1.0, initial-scale=1\" /> </head> <body> <h1>Recebe Valor do Arduino:</h1> <h3>Nível: <span id=\"nivel\"></span></h3> <h3>Temperatura: <span id=\"temp\"></span></h3> <h1>Enviar valor para Arduino</h1> <input placeholder=\"Digite um Texto\" id=\"input1\"> <button onClick=\"setValor(1)\">Enviar</button> <p>Varlos Enviado: <span id=\"valorEnviado\"></span></p> <script> function getAllStatus() { console.log(\"Entrou no getAll\"); var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { var obj = JSON.parse(this.responseText); document.getElementById(\"nivel\").innerHTML = obj.data[0].nivel; document.getElementById(\"temp\").innerHTML = obj.data[1].temperatura; console.log(obj) }; }; xhttp.open(\"GET\", \"data\", true); xhttp.send(); }; function setValor(valor) { console.log(\"Set Valor\"); var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"valorEnviado\").innerHTML = this.responseText; } }; var valorInput = document.getElementById(\"input1\").value; console.log(valorInput); xhttp.open(\"GET\", \"setValue?valueInput=\"+valorInput, true); xhttp.send(); }; getAllStatus(); </script> </body> </html>");

}

void loop() {
  httpFreedom.handleClient();

}
