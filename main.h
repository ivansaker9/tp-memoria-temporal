#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/servidor.h"
#include "../../utils/src/utils.h"

typedef struct {
    String puerto_escucha;
    String tam_memoria;
    String tam_pagina;
    String path_intrucciones;
    String retardo_respuesta;
} t_memoria_config;

typedef enum {
    SET,
    IO_GEN_SLEEP,
    SUM,
    SUB,
    EXIT
} t_tipo_instruccion;

typedef struct {
    t_tipo_instruccion tipo;
    char registro1[10]; // Para instrucciones tipo SET, SUM y SUB
    char registro2[10]; // Para instrucciones tipo SUM y SUB
    int valor;          // Para instrucciones tipo SET
    char dispositivo[20]; // Para instrucciones tipo IO_GEN_SLEEP
    int tiempo;         // Para instrucciones tipo IO_GEN_SLEEP
} t_instruccion;



/**
* @fn    Carga la configuración en la estructura memoria_config
* @brief Con el archivo config abierto solicita memoria y lo carga en la estructura memoria_config
*/
t_memoria_config* load_memoria_config(t_config* config);

#endif
