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
        stdout_recibir_direccion(conexion_kernel, conexion_memoria);

    close(conexion_kernel);
    close(conexion_memoria);
}

void io_stdout_write(int direccion, int cant_caracteres, int fd_memoria){

    int total_size = 2 * sizeof(int);

    payload_t *buffer = payload_create(total_size);

    // cant_direcciones
    payload_add(buffer, &direccion, sizeof(int));
    payload_add(buffer, &cant_caracteres, sizeof(int));

    paquete_t *paquete_a_enviar = crear_paquete(IO_STDOUT_WRITE, buffer);
    
    if(enviar_paquete(fd_memoria, paquete_a_enviar) != OK){
        
        log_error(extra_logger, "No se pudo enviar el paquete a memoria");
        
        payload_destroy(buffer);
        liberar_paquete(paquete_a_enviar);
    }

    paquete_t *paquete = recibir_paquete(fd_memoria);

    if(paquete == NULL)
        exit(EXIT_FAILURE);
    if(paquete->operacion != IO_STDOUT_WRITE){
        log_error(extra_logger, "Error en recibir el paquete de memoria");
        return;
    }
    
    //leemos el paquete
    char* texto = payload_read_string(paquete->payload);
    // Mostramos la dirección recibida por pantalla
    printf("Texto recibido de memoria: %s\n", texto);
    //Respetamos el tiempo unidad de trabajo
    TIEMPO_UNIDAD_DE_TRABAJO(interfaz_config->tiempo_u_trabajo);

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
}

void stdout_recibir_direccion(int fd_kernel, int fd_memoria){
    
    paquete_t *paquete = recibir_paquete(fd_kernel);

    if(paquete == NULL)
        exit(EXIT_FAILURE);
    if(paquete->operacion != IO_STDOUT_WRITE){
        log_error(extra_logger, "Error operacion invalida");
        return;
    }
    
    //leemos el paquete
    int direccion;
    int cant_caracteres;
    payload_read(paquete->payload, &direccion, sizeof(int));
    payload_read(paquete->payload, &cant_caracteres, sizeof(int));


    //La mando para mostrar por pantalla
    io_stdout_write(direccion, cant_caracteres, fd_memoria);

    //Libero memoria
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
}








