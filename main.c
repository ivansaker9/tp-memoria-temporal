#include "main.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
    t_config* config = iniciar_config("memoria.config");
    t_memoria_config* memoria_config = load_memoria_config(config);

    log_info(logger, "Archivo de configuración cargado correctamente");

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    int memoria_server = escuchar_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, logger);

    // La MEMORIA espera que la CPU se conecte
    int conexion_cpu = esperar_conexion_de(CPU_CON_MEMORIA, memoria_server);

    if(conexion_cpu < 0)
        log_error(logger, "MODULO CPU NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_info(logger, "MODULO CPU CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // La MEMORIA espera que el KERNEL se conecte
    int conexion_kernel = esperar_conexion_de(KERNEL_CON_MEMORIA, memoria_server);

    if(conexion_kernel < 0)
        log_error(logger, "MODULO KERNEL NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_info(logger, "MODULO KERNEL CONECTO CON LA MEMORIA EXITOSAMENTE!");


    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(memoria_server);

    sleep(30);

    // Cierro todos lo archivos y libero los punteros usados
    close(fd_memoria_server);
    log_destroy(logger);
    config_destroy(config);
    free(memoria_config);
    
    return OK;
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

void *thread_aceptar_clientes(void *socket_servidor) {
    aceptar_clientes(*(int*)socket_servidor);
    free(socket_servidor);
    pthread_exit(NULL);
    //recibimos el path del kernel
    char* path_instrucciones = malloc(10*sizeof(char));
    recv(conexion_kernel, path_instrucciones);

    int* program_counter;
    recv(conexion_cpu, program_counter);

    //La convertimos y enviamos a la cpu
    leer_y_convertir_instrucciones(path_instrucciones, conexion_cpu, program_counter);
    return NULL;
}

void leer_y_convertir_instrucciones(char* path_instrucciones, int socket_cpu, program_counter) {
    FILE *archivo = fopen(path_instrucciones, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    char line[100];
    t_instruccion instruccion;

    while (fgets(line, sizeof(line), archivo)) {

        line[strcspn(line, "\n")] = 0;  // Eliminar el salto de línea
        instruccion = convertir_instruccion(line);
        if (program_counter) > 0) {
        enviar_instruccion_a_cpu(socket_cpu, &instruccion);
        }
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
    int size;
    void* buffer = serializar_instruccion(instruccion, &size);

    send(socket_cpu, buffer, size, 0);

    free(buffer);
}



void* serializar_instruccion(t_instruccion* instruccion, int* size) {
    int offset = 0;
    *size = sizeof(t_tipo_instruccion) + sizeof(instruccion->registro1) + sizeof(int) + sizeof(instruccion->dispositivo) + sizeof(int) + sizeof(instruccion->registro2);

    void* buffer = malloc(*size);

    memcpy(buffer + offset, &(instruccion->tipo), sizeof(t_tipo_instruccion));
    offset += sizeof(t_tipo_instruccion);

    switch (instruccion->tipo) {
        case SET:
            memcpy(buffer + offset, instruccion->registro1, sizeof(instruccion->registro1));
            offset += sizeof(instruccion->registro1);
            memcpy(buffer + offset, &(instruccion->valor), sizeof(int));
            offset += sizeof(int);
            break;
        case IO_GEN_SLEEP:
            memcpy(buffer + offset, instruccion->dispositivo, sizeof(instruccion->dispositivo));
            offset += sizeof(instruccion->dispositivo);
            memcpy(buffer + offset, &(instruccion->tiempo), sizeof(int));
            offset += sizeof(int);
            break;
        case SUM:
        case SUB:
            memcpy(buffer + offset, instruccion->registro1, sizeof(instruccion->registro1));
            offset += sizeof(instruccion->registro1);
            memcpy(buffer + offset, instruccion->registro2, sizeof(instruccion->registro2));
            offset += sizeof(instruccion->registro2);
            break;
        case EXIT:
            break;
    }

    return buffer;
}


