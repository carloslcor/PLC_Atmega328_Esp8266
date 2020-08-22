#ifndef _CLCOR_IO_H_ 
#define _CLCOR_IO_H_

#define ARRAY_LENGTH( arr ) sizeof( arr ) / sizeof( arr[0] )

typedef enum _clcor_io_type_t {
    CLCOR_IO_TYPE_DIGITAL = 0,
    CLCOR_IO_TYPE_ANALOG,
    CLCOR_IO_TYPE_T__NUM
} clcor_io_type_t;

#define IO_DESC_T_ALIAS_LENGTH (16)
typedef struct _io_desc {
    char alias[IO_DESC_T_ALIAS_LENGTH];//Descricao do IO(Motor da Bomba)
    clcor_io_type_t type;
    uint8_t num; // Numero do pino digital ou analogico
    uint8_t iomode; // Modo de operacao, se é entrada ou saida, inputm output ou input_pullup
    uint16_t value; // Valor do pino, digital ou analocido 0 ou 1, 0 - 1023.
} clcor_io_desc_t;

const char *clcor_io_type_strs[CLCOR_IO_TYPE_T__NUM] = {
    "digital",
    "analogico"
};

const char *clcor_iomode_strs[] = {
    "input",
    "output",
    "input_pullup",
};

void clcor_dump(clcor_io_desc_t *io_desc){
    Serial.println("{");
    Serial.print("alias: ");
    Serial.println(io_desc->alias);
    Serial.print("type: ");
    Serial.print(clcor_io_type_strs[io_desc->type]);
    Serial.println();
    Serial.print("num: ");
    Serial.println(io_desc->num);
    Serial.print("iomode: ");
    Serial.println(clcor_iomode_strs[io_desc->iomode]);
    Serial.print("value: ");
    Serial.println(io_desc->value);
    Serial.println("},");
}

clcor_io_desc_t app_ios[] ={
    {"rele01", CLCOR_IO_TYPE_DIGITAL, 10, INPUT_PULLUP, 0},
    {"rele02", CLCOR_IO_TYPE_DIGITAL, 12, INPUT_PULLUP, 0}
    // {"rele03", CLCOR_IO_TYPE_DIGITAL, 13, OUTPUT, 0}
};

#endif /*_CLCOR_IO_H_*/
