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
    char registro1[20];
    int valor;
    char dispositivo[20];
    int tiempo;
    char registro2[20];
} t_instruccion;

void* serializar_instruccion(t_instruccion* instruccion, int* size);
void enviar_instruccion_a_cpu(int socket_cpu, t_instruccion* instruccion);
t_instruccion convertir_instruccion(char* line);
void leer_y_convertir_instrucciones(const char* path_instrucciones, int socket_cpu);
t_memoria_config* load_memoria_config(t_config* config);

/**
* @fn    Carga la configuración en la estructura memoria_config
* @brief Con el archivo config abierto solicita memoria y lo carga en la estructura memoria_config
*/
t_memoria_config* load_memoria_config(t_config* config);

#endif
