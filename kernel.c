//funcion de prueba stdout
void send_io_stdout_write(int socket) {

    int cant_caracteres = 12;
    int cant_direcciones = 4;

    int tam = 2 * sizeof(int);

    payload_t *payload = payload_create(tam);

    // cantidad de direcciones
    payload_add(payload, &cant_direcciones, sizeof(uint32_t));

    // cantidad de caracteres
    payload_add(payload, &cant_caracteres, sizeof(int));

    paquete_t *paquete = crear_paquete(IO_STDOUT_WRITE, payload);

    if(enviar_paquete(socket, paquete) != OK){
        printf("fallo al enviar el paquete a stdout");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload);
    liberar_paquete(paquete);
}
