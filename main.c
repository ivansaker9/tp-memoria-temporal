#include "main.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
    t_config* config = iniciar_config("memoria.config");
    t_memoria_config* memoria_config = load_memoria_config(config);

    log_info(logger, "Archivo de configuración cargado correctamente");

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    int fd_memoria_server = modulo_escucha_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, logger);

    // Acepto clientes en un thread aparte asi no frena la ejecución del programa
    pthread_t thread_memoria;
    atender_conexiones_al_modulo(&thread_memoria, fd_memoria_server);

    pthread_join(thread_memoria, NULL);

    //recibimos el path del kernel y lo convertimos
    char* path_instrucciones = recv(socket_cliente, size, sizeof(*string), MSG_WAITALL);
    convertir_instruccion(path_instrucciones);

    // Cierro todos lo archivos y libero los punteros usados
    close(fd_memoria_server);
    log_destroy(logger);
    config_destroy(config);
    free(memoria_config);
    
    return EXIT_OK;
}

t_memoria_config* load_memoria_config(t_config* config) {
    
    t_memoria_config* memoria_config = malloc(sizeof(t_memoria_config));
    
    if(memoria_config == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    memoria_config->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    memoria_config->tam_memoria = config_get_string_value(config,"TAM_MEMORIA");
    memoria_config->tam_pagina = config_get_string_value(config,"TAM_PAGINA");
    memoria_config->path_intrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    memoria_config->retardo_respuesta = config_get_string_value(config,"RETARDO_RESPUESTA");

    return memoria_config;
}


void leer_instruccion(const char* path_instrucciones) {
    // Abrimos el archivo para leer
    FILE *archivo = fopen(path_instrucciones, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    char line[100];
    t_instruccion instruccion;

    while (!feof(archivo)) {
        // Eliminamos el salto de línea al final de la cadena
        line[strcspn(line, "\n")] = 0;

        // Parseamos la instrucción
        instruccion = convertir_instruction(line);
        
        // Aquí puedes enviar la instrucción parseada a la CPU
        // Por ejemplo, imprimirla para ver que se parseó correctamente
        switch (instruccion.tipo) {
            case SET:
                printf("SET %s %d\n", instruccion.registro, instruccion.valor);
                break;
            case IO_GEN_SLEEP:
                printf("IO_GEN_SLEEP %s %d\n", instruccion.dispositivo, instruccion.tiempo);
                break;
            case EXIT:
                printf("EXIT\n");
                break;
        }
        
        // Aquí llamarías a la función para enviar la instrucción a la CPU
        // enviar_instruccion_a_cpu(instruccion);
    }

    fclose(archivo);
}


    t_instruccion Convertir_instruction(char* line) {
        t_instruccion instruccion;
        char tipo[20];

        sscanf(line, "%s", tipo);

        if (strcmp(tipo, "SET") == 0) {
            instruccion.tipo = SET;
            sscanf(line, "%*s %s %d", instruccion.registro, &instruccion.valor);
        } else if (strcmp(tipo, "IO_GEN_SLEEP") == 0) {
            instruccion.tipo = IO_GEN_SLEEP;
            sscanf(line, "%*s %s %d", instruccion.dispositivo, &instruccion.tiempo);
        } else if (strcmp(tipo, "EXIT") == 0) {
            instruccion.tipo = EXIT;
        }

        return instruccion;
    }
