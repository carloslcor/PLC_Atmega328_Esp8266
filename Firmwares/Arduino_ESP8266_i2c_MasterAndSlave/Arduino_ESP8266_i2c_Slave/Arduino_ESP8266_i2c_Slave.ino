/*
I2C Slave
Author: Pedro Minatel
Gravar esse código no Arduino.
//https://imasters.com.br/iot-makers/comunicando-o-arduino-com-o-esp8266-por-i2c
*/

#include <Wire.h>
#include "/Users/carlos/Google Drive/Freedom Engenharia/Github/PLC Web/plc_web/Firmwares/lib/inc/clcor_io.h"

#define ARRAY_LENGTH( arr ) sizeof( arr ) / sizeof( arr[0] )

#define MAX_LENGTH 20
#define SWITCH_PIN (12)
#define TXR_BUFFER_SIZE (32)
#define I2C_SLAV_ADDR  (2)

static uint8_t txr_buffer[TXR_BUFFER_SIZE];
static size_t txr_size = 0;
static bool response_size_sent;
static String acumulador;

static clcor_io_desc_t app_ios[] ={
    {"rele01", CLCOR_IO_TYPE_DIGITAL, SWITCH_PIN, INPUT_PULLUP, 0}
};

//Dado recebido do mastre
//uint8_t rec_value = 0;
void setup(){
    size_t i=0;

    acumulador.reserve(MAX_LENGTH + 1);  // Define o tamanho máximo do buffer (+ 1 por causa do \0 no final
    //Inicia como escravo no endereço 2
    Wire.begin(I2C_SLAV_ADDR);
    //Callback para dados recebidos do mestre
    Wire.onReceive(receiveCallback);
    //Callback para requisições recebidas do mestre
    Wire.onRequest(requestCallback);
    //LED embarcado na placa
    pinMode(LED_BUILTIN, OUTPUT);
    //Serial para debug
    Serial.begin(9600);
    Serial.println("Arduino Inicializado:");

    Serial.println("#### IO Setup");
    for(i = 0; i < ARRAY_LENGTH(app_ios); i++){
        Serial.print(i, DEC);
        Serial.print(" => ");
        clcor_dump(&app_ios[i]);
    }
}
void loop() {
}

//Callback para dados recebidos do mestre
void receiveCallback(int bytes){
    size_t i=0;
    //Se tiver dados recebidos
    if(Wire.available() != 0){
        Serial.print("comando recebido: ");
        for(int i = 0; i < bytes; i++){
            //Lê os dados
            //rec_value = Wire.read();
            acumulador.concat((char)Wire.read());
        }
        acumulador.toUpperCase();    // Converte toda a String para maiusculo
        acumulador.trim();           // Tira os espacos antes e depois
        Serial.print(acumulador);
        if(acumulador == "STATUS"){
            Serial.println();
            txr_size = 0;
            for(i = 0; i < ARRAY_LENGTH(app_ios); i++){
                //type
                txr_buffer[txr_size] = app_ios[i].type;
                txr_size += 1;
                //num
                txr_buffer[txr_size] = app_ios[i].num;
                txr_size += 1;
                //iomode
                txr_buffer[txr_size] = app_ios[i].iomode;
                txr_size += 1;
                //value
                app_ios[i].value = digitalRead(app_ios[i].num);
                txr_buffer[txr_size] = (uint8_t) (app_ios[i].value >> 8) & 0xFF;
                txr_size += 1;
                txr_buffer[txr_size] = (uint8_t) app_ios[i].value & 0xFF;
                txr_size += 1;
            }
            response_size_sent = false;
        } else {
            Serial.println(" (desconhecido)");
        }
        acumulador = ""; // Limpa a String para comecar a armazenar de novo
    }
}
//Callback para requisições recebidas do mestre
void requestCallback(void){
//Retorna para o mestre o valor recebido
    size_t i;
    if(!response_size_sent){
        Serial.println("mandando header");
        Wire.write(txr_size);
        response_size_sent = true;
    } else {
        Serial.println("mandando payload");
        for(i = 0; i < txr_size; i++){
            Wire.write(txr_buffer[i]);
        }
        txr_size = 0;
    }
}