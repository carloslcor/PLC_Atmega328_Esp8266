/*
I2C Master
Author: Pedro Minatel
Gravar esse código no ESP8266.

//https://imasters.com.br/iot-makers/comunicando-o-arduino-com-o-esp8266-por-i2c
*/

#include <Wire.h>
#include "/Users/carlos/Google Drive/Freedom Engenharia/Github/PLC Web/plc_web/Firmwares/lib/inc/clcor_io.h"

#define I2C_SLAV_ADDR  (2)
#define TXR_BUFFER_SIZE (32)

static uint8_t txr_buffer[TXR_BUFFER_SIZE];

static clcor_io_desc_t app_ios[] ={
    {"rele01", (clcor_io_type_t)0, 0, 0, 0}
};

void setup() {
    Wire.begin();
    //LED embarcado na placa
    pinMode(LED_BUILTIN, OUTPUT);
    //Serial para debug
    Serial.begin(9600);
    Serial.println();
    Serial.println("ESP8266 Inicializado:");

}

void loop() {
    size_t requested_bytes;
    size_t received_bytes;
    size_t i = 0;
    Serial.println("Enviando");
    //Inicia a transmissão para o endereço 2
    Wire.beginTransmission(I2C_SLAV_ADDR);
    //Escreve no barramento o estado do LED
    Wire.write("status");
    //Termina a transmissão
    Wire.endTransmission();    // stop transmitting
    delay(10);
    Serial.print("recebendo resposta...");

    //Espera a resposta do escravo
    requested_bytes = 1;
    received_bytes = Wire.requestFrom(I2C_SLAV_ADDR, requested_bytes);
    //Se houver resposta
    if(received_bytes > 0) {
        Serial.println();
        Serial.print("bytes recebidos 1: ");
        Serial.print(received_bytes, DEC);
        Serial.print("/");
        Serial.print(requested_bytes, DEC);
        Serial.print(" => ");
        txr_buffer[0] = Wire.read();
        Serial.println(txr_buffer[0], HEX);

        requested_bytes = txr_buffer[0];
        received_bytes = Wire.requestFrom(I2C_SLAV_ADDR, requested_bytes);
        Serial.println();
        Serial.print("bytes recebidos 2: ");
        Serial.print(received_bytes, DEC);
        Serial.print("/");
        Serial.print(requested_bytes, DEC);
        Serial.print(" => ");
        for(i = 0; i < received_bytes; i++){
            //Lê o byte de resposta
            txr_buffer[i] = Wire.read();
            Serial.print(txr_buffer[i], HEX);
            Serial.print(", ");
        }
        Serial.println("");
        i = 0;
        //type
        app_ios[0].type = (clcor_io_type_t) txr_buffer[i];
        i += 1;
        //num
        app_ios[0].num = txr_buffer[i];
        i += 1;
        //iomode
        app_ios[0].iomode = txr_buffer[i];
        i += 1;
        //value
        app_ios[0].value = 0;
        app_ios[0].value |= txr_buffer[i] << 8;
        i += 1;
        app_ios[0].value |= txr_buffer[i] & 0xFF;
        i += 1;

        Serial.print(0, DEC);
        Serial.print(" => ");
        clcor_dump(&app_ios[0]);
    } else {
        //Se caso a resposta do escravo for diferente
        Serial.println(" sem resposta =/");
    }

    delay(1000);
}
