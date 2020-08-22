/*
I2C Master
Author: Pedro Minatel
Gravar esse código no ESP8266.

//https://imasters.com.br/iot-makers/comunicando-o-arduino-com-o-esp8266-por-i2c
*/

#include <Wire.h>
#include "/Users/carlos/Google Drive/Freedom Engenharia/Github/PLC Web/plc_web/Firmwares/lib/inc/clcor_io.h"
#include <LiquidCrystal_I2C.h>

#define I2C_SLAV_ADDR  (2)
#define TXR_BUFFER_SIZE (32)

static uint8_t txr_buffer[TXR_BUFFER_SIZE];


LiquidCrystal_I2C lcd(0x3F,16,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
    Wire.begin();
    //LED embarcado na placa
    pinMode(LED_BUILTIN, OUTPUT);
    //Serial para debug
    Serial.begin(9600);
    Serial.println();
    Serial.println("ESP8266 Inicializado:");

    lcd.init();                      // initialize the lcd 
    lcd.backlight();

    // lcd.printstr("qweqe");
    lcd.setCursor(0,0);
    lcd.print("    linha 1     ");
    lcd.setCursor(0,1);
    lcd.print("    linha 2     ");
    lcd.setCursor(-4,2);
    lcd.print("    linha 3     ");
    lcd.setCursor(-4,3);
    lcd.print("    linha 4     ");
}

void loop() {
    size_t requested_bytes;
    size_t received_bytes;
    size_t i = 0, j;
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

        delay(10);

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
        for(j = 0; j < ARRAY_LENGTH(app_ios); j++){
            //type
            app_ios[j].type = (clcor_io_type_t) txr_buffer[i];
            i += 1;
            //num
            app_ios[j].num = txr_buffer[i];
            i += 1;
            //iomode
            app_ios[j].iomode = txr_buffer[i];
            i += 1;
            //value
            app_ios[j].value = 0;
            app_ios[j].value |= txr_buffer[i] << 8;
            i += 1;
            app_ios[j].value |= txr_buffer[i] & 0xFF;
            i += 1;

            Serial.print(j, DEC);
            Serial.print(" => ");
            clcor_dump(&app_ios[j]);
        }

    } else {
        //Se caso a resposta do escravo for diferente
        Serial.println(" sem resposta =/");
    }

    delay(1000);
}
