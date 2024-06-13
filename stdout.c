#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"
#include "../../utils/src/serializacion.h"
#include "main.h"

extern t_interfaz_config* interfaz_config;
extern t_log* logger;
extern t_log* extra_logger;
extern int conexion_memoria, conexion_kernel;

void interfaz_stdout(String nombre){
    //me conecto a memoria
    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDOUT_CON_MEMORIA, extra_logger);
    //me conecto al kernel para mandarle el nombre
    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDOUT_CON_KERNEL, extra_logger);
    //enviamos nombre de la interfaz
    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1)
        recibir_direccion(conexion_memoria);

    close(conexion_kernel);
    close(conexion_memoria);
}

void io_stdout_write(char* direccion){
    
    // Mostramos la dirección recibida por pantalla
    printf("Dirección recibida: %s\n", direccion);
    //Respetamos el tiempo unidad de trabajo
    TIEMPO_UNIDAD_DE_TRABAJO(interfaz_config->tiempo_u_trabajo);
}

void stdout_recibir_direccion(int fd_memoria){
    
    paquete_t *paquete = recibir_paquete(fd_memoria);

    if(paquete == NULL)
        exit(EXIT_FAILURE);
    if(paquete->operacion != IO_STDOUT_WRITE){
        log_error(extra_logger, "Error en recibir la direccion de memoria");
        return;
    }
    
    //leemos el paquete
    char* direccion = malloc(10 * sizeof(char));
    payload_read(paquete->payload, direccion, 10 * sizeof(char));

    //La mando para mostrar por pantalla
    io_stdout_write(direccion);

    //Libero memoria
    free(direccion);
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
}

// KERNEL
void send_io_stdout_write(int socket) {

    int cant_caracteres = 12;
    int cant_direcciones = 4;
    int direcciones[] = {88, 56, 45, 22};

    int tam = sizeof(int) + cant_direcciones * sizeof(int) + sizeof(int);

    payload_t *payload = payload_create(tam);

    // cantidad de direcciones
    payload_add(payload, &cant_direcciones, sizeof(uint32_t));

    // direcciones
    for(int i = 0; i < cant_direcciones; i++) {
        payload_add(payload, &direcciones[i], sizeof(int));
    }

    // cantidad de caracteres
    payload_add(payload, &cant_caracteres, sizeof(int));

    paquete_t *paquete = crear_paquete(IO_STDIN_READ, payload);

    if(enviar_paquete(socket, paquete) != OK){
        printf("fallo al enviar el paquete a stdout");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload);
    liberar_paquete(paquete);
}







