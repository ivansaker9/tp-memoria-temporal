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


void leer_y_convertir_instrucciones(const char* path_instrucciones, int socket_cpu) {
    FILE *archivo = fopen(path_instrucciones, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    char line[100];
    t_instruccion instruccion;

    while (fgets(line, sizeof(line), archivo)) {
        // Eliminamos el salto de línea al final de la cadena
        line[strcspn(line, "\n")] = 0;

        // Parseamos la instrucción
        instruccion = convertir_instruccion(line);
        
        // Enviar la instrucción a la CPU
        enviar_instruccion_a_cpu(socket_cpu, &instruccion);
    }

    fclose(archivo);
}



t_instruccion convertir_instruccion(char* line) {
    t_instruccion instruccion;
    char tipo[20];
    sscanf(line, "%s", tipo);

    if (strcmp(tipo, "SET") == 0) {
        instruccion.tipo = SET;
        sscanf(line, "%*s %s %d", instruccion.registro1, &instruccion.valor);
    } else if (strcmp(tipo, "IO_GEN_SLEEP") == 0) {
        instruccion.tipo = IO_GEN_SLEEP;
        sscanf(line, "%*s %s %d", instruccion.dispositivo, &instruccion.tiempo);
    } else if (strcmp(tipo, "SUM") == 0) {
        instruccion.tipo = SUM;
        sscanf(line, "%*s %s %s", instruccion.registro1, instruccion.registro2);
    } else if (strcmp(tipo, "SUB") == 0) {
        instruccion.tipo = SUB;
        sscanf(line, "%*s %s %s", instruccion.registro1, instruccion.registro2);
    } else if (strcmp(tipo, "EXIT") == 0) {
        instruccion.tipo = EXIT;
    }

    return instruccion;
}

void enviar_instruccion_a_cpu(int socket_cpu, t_instruccion* instruccion) {
    // Aquí serializamos la estructura para enviarla a través del socket
    // Por simplicidad, se usa `send` directamente, pero puedes crear una función de serialización más compleja si es necesario

    // Enviar tipo de instrucción
    send(socket_cpu, &(instruccion->tipo), sizeof(instruccion->tipo), 0);

    // Enviar campos según el tipo de instrucción
    switch (instruccion->tipo) {
        case SET:
            send(socket_cpu, instruccion->registro1, sizeof(instruccion->registro1), 0);
            send(socket_cpu, &(instruccion->valor), sizeof(instruccion->valor), 0);
            break;
        case IO_GEN_SLEEP:
            send(socket_cpu, instruccion->dispositivo, sizeof(instruccion->dispositivo), 0);
            send(socket_cpu, &(instruccion->tiempo), sizeof(instruccion->tiempo), 0);
            break;
        case SUM:
        case SUB:
            send(socket_cpu, instruccion->registro1, sizeof(instruccion->registro1), 0);
            send(socket_cpu, instruccion->registro2, sizeof(instruccion->registro2), 0);
            break;
        case EXIT:
            // No hay campos adicionales para enviar
            break;
    }
}

