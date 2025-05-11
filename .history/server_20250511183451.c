#include "server_utils.h"


extern Usuario *usuarios;
extern int num_usuarios;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;



int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: ./servidor -p <puerto>\n");
        return -2; // Error de argumentos
    }

    int puerto = atoi(argv[2]); // Define el puerto, que saca de la linea de comandos
    if (puerto <= 0 || puerto > 65535) {
        fprintf(stderr, "Puerto inválido. Debe estar entre 1 y 65535.\n");
        return -2; // Error de  valor de puerto invalido
    }
    int sd = socket(AF_INET, SOCK_STREAM, 0); //usa IPv4 y por tcp (un socket es parecido a un descriptor de archivo fd)

    struct sockaddr_in direccion;
    direccion.sin_family = AF_INET;          // Tipo de dirección: IPv4
    direccion.sin_port = htons(puerto);      // Puerto del servidor (en orden de red) host to network
    direccion.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones desde cualquier IP

    bind(sd, (struct sockaddr *)&direccion, sizeof(direccion)); //asociamos el socket a la ip y puerto local

    listen(sd,10); // "encendemos" el socket para recibir conexiones externas, ponemos 10 como maximo de cola
    char ip_local[16]; //len = 16 para IPv4
    obtener_ip_local(ip_local, sizeof(ip_local)); //obtenemos la ip local
    printf("s> init server %s:%d\ns> ", ip_local, puerto); // mensaje de inicio
    fflush(stdout);

    while (1) {
        struct sockaddr_in cliente;
        socklen_t tam = sizeof(cliente);
        int *cliente_sd = malloc(sizeof(int));

        // espera a la conexion y devuelve un nuevo socket para hablar con ese cliente
        *cliente_sd = accept(sd, (struct sockaddr *)&cliente, &tam); 
        if (*cliente_sd < 0) { //error de cliente
            perror("accept");
            free(cliente_sd);
            continue;
        }

        pthread_t hilo;
        if (pthread_create(&hilo, NULL, handle_client, cliente_sd) != 0) {
            perror("pthread_create"); //error al crear thread
            close(*cliente_sd);
            free(cliente_sd);
            continue;
        }
        pthread_detach(hilo); // El hilo se limpiará solo al terminar
    }

    close(sd); //cerramos el socket
    return 0;
}

/*
cada cliente que se conecte:
	1.	Envía un comando (por ejemplo "REGISTER")
	2.	El servidor lo lee desde el socket (read())
	3.	Procesa ese comando (empezamos por "REGISTER")
	4.	Responde con un código: 0 (OK), 1 (USERNAME IN USE), 2 (ERROR)
*/

void *handle_client(void *arg) {
    // printf("DEBUG: cliente conectado\n"); 

    int cliente_sd = *(int *)arg;
    free(arg);

    char comando[256];
    if (readLine(cliente_sd, comando, sizeof(comando)) < 1) {
        perror("Error de lectura ");
        close(cliente_sd);
        return NULL;
    }


    // según que hayamos escrito en la terminal...
    if (strcmp(comando, "REGISTER") == 0) {
        register_client(cliente_sd);
    } else if (strcmp(comando, "PUBLISH") == 0) {
        publish(cliente_sd);
    } else if (strcmp(comando, "CONNECT") == 0) {
        connect_user(cliente_sd);
    } else if (strcmp(comando, "LIST_USERS") == 0) {
        list_users(cliente_sd);
    } else if (strcmp(comando, "LIST_CONTENT") == 0) {
        list_content(cliente_sd);
    } else if (strcmp(comando, "GET_FILE") == 0) {
        get_file(cliente_sd);
    } else if (strcmp(comando, "DISCONNECT") == 0) {
        disconnect_user(cliente_sd);
    } else if (strcmp(comando, "UNREGISTER") == 0) {
        unregister_user(cliente_sd);
    } else if (strcmp(comando, "DELETE") == 0) {
        delete_file(cliente_sd);
    }

    // Ejemplo representativo de uso real de sendMessage()
    // Al final de handle_client(), justo antes de cerrar el socket
    // sendMessage(cliente_sd, "Comando procesado correctamente");

    close(cliente_sd);
    return NULL;
}

void *register_client(int cliente_sd) {
    char nombre[256];
    uint8_t resultado = 2;
    // printf("DEBUG: esperando nombre del usuario...\n");
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    // printf("DEBUG: nombre recibido: '%s'\n", nombre);
    resultado = registrar_usuario(nombre);
    log_operation("REGISTER", nombre);
    send(cliente_sd, &resultado, 1, 0);
    return NULL;
}

void *publish(int cliente_sd) {
    char nombre[256];
    char path[256];
    char descripcion[256];
    uint8_t resultado = 4;

    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    log_operation("PUBLISH", nombre);
    Usuario *u = buscar_usuario(nombre);
    // comprobando si existe
    if (u == NULL) {
        resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    // comprobando si esta conectado
    if (!u->conectado){
        resultado = 2;
        send(cliente_sd, &resultado, 1, 0);
    }
    if (readLine(cliente_sd, path, sizeof(path)) < 1) {
        perror("Error leyendo el path");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    if (readLine(cliente_sd, descripcion, sizeof(descripcion)) < 1) {
        perror("Error leyendo la descripción");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    //comprobamos si ya fue publicado antes
    for(int i = 0; i < u->num_ficheros; i++){
        if (strcmp(u->ficheros[i].path, path) == 0){
            resultado = 3;
            send(cliente_sd, &resultado, 1, 0);
            return NULL;
        }
    }
    // creamos el nuevo fichero
    File *nuevo = malloc((u->num_ficheros + 1) * sizeof(File));
    if (nuevo == NULL) {
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    memcpy(nuevo, u->ficheros, u->num_ficheros * sizeof(File));
    free(u->ficheros);
    u->ficheros = nuevo;
    strcpy(u->ficheros[u->num_ficheros].path, path);
    strcpy(u->ficheros[u->num_ficheros].descripcion, descripcion);
    u->num_ficheros++;
    resultado = 0;
    send(cliente_sd, &resultado, 1, 0); 
    return NULL;
}
/*
proceso:
	•	Leer nombre
	•	Leer puerto
	•	Buscar usuario
	•	Validar si está o no conectado
    •   Obtener ip
*/

void *connect_user(int cliente_sd) {
    char nombre[256];
    int puerto;
    uint8_t resultado = 3;

    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    log_operation("CONNECT", nombre);


    // recv espera un int
    if (recv(cliente_sd, &puerto, sizeof(puerto), 0) <= 0) {
        perror("Error leyendo el puerto del usuario");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    // Leer la IP que el cliente ha enviado explícitamente
    char ip_cliente[64];
    if (readLine(cliente_sd, ip_cliente, sizeof(ip_cliente)) < 1) {
        perror("Error leyendo IP del cliente");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    // comprobamos si existe 
    if (u == NULL) {
        resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    // comprobamos si esta conectado
    if (u->conectado){
        resultado = 2;
        send(cliente_sd, &resultado, 1, 0);
    }

    strncpy(u->ip, ip_cliente, sizeof(u->ip));
    u->ip[sizeof(u->ip) - 1] = '\0';
    u->puerto = puerto;
    u->conectado = 1;

    resultado = 0;
    send(cliente_sd, &resultado, 1, 0);
    return NULL;
}


void *list_users(int cliente_sd) {
    char nombre[256];
    uint8_t resultado = 4;
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1 ){
        send(cliente_sd, &resultado, 1, 0);
        perror("Error leyendo nombre en LIST_CONTENT");
        return NULL;
    }
    log_operation("LIST_USERS", nombre);


    Usuario *u = buscar_usuario(nombre);
    // comprbamos si quien pregunta existe
    if (!u ){
        resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    // comprobamos si quien pregunta esta conectado 
    if(!u->conectado) {
        resultado = 2;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    int n = 0;
    for(int i = 0; i < num_usuarios; i++){
        if (usuarios[i].conectado) { n++; }
    }

    resultado = 0;
    send(cliente_sd, &resultado, 1, 0);
    char n_clientes[12];
    snprintf(n_clientes, sizeof(n_clientes), "%d", n);
    sendMessage(cliente_sd, n_clientes);

    for (int i = 0; i < num_usuarios; i++) {
        if (usuarios[i].conectado) {
            //nombre
            sendMessage(cliente_sd, usuarios[i].nombre);
            // direccion IP
            sendMessage(cliente_sd, usuarios[i].ip);
            // puerto
            char puerto_ascii[12];
            snprintf(puerto_ascii, sizeof(puerto_ascii), "%d", usuarios[i].puerto);
            sendMessage(cliente_sd, puerto_ascii);
        }
    }
    return NULL;
}

// Devuelve la lista de ficheros publicados por un usuario conectado
void *list_content(int cliente_sd) {
    char nombre[256]; // quien pregunta
    char remoto[256]; // de quien se pregunta
    uint8_t resultado = 4;
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1 || readLine(cliente_sd, remoto, sizeof(remoto)) < 1) {
        send(cliente_sd, &resultado, 1, 0);
        perror("Error leyendo nombre en LIST_CONTENT");
        return NULL;
    }
    log_operation("LIST_CONTENT", nombre);


    Usuario *u = buscar_usuario(nombre);
    // comprbamos si quien pregunta existe
    if (!u ){
        resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    // comprobamos si quien pregunta esta conectado 
    if(!u->conectado) {
        resultado = 2;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    Usuario *r = buscar_usuario(remoto);
    // comprbamos si de quien pregunta existe
    if (!r ){
        resultado = 3;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    resultado = 0;
    send(cliente_sd, &resultado, 1, 0);

    char n_fich[512];
    snprintf(n_fich, sizeof(n_fich), "%d", r->num_ficheros);
    sendMessage(cliente_sd, n_fich);
    
    for (int i = 0; i < r->num_ficheros; i++) {
        sendMessage(cliente_sd, r->ficheros[i].path); // Comentado, no requerido por el protocolo
    }
    return NULL;
}
// ESTA CREO QUE ES ENTRE CLIENTES ASI QUE NO DEBERIA ESTAR AQUI
void *get_file(int cliente_sd) {
    char nombre[256], fichero[256], local[256];

    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1 ||
        readLine(cliente_sd, fichero, sizeof(fichero)) < 1 ||
        readLine(cliente_sd, local, sizeof(local)) < 1) {
        perror("Error leyendo datos en GET_FILE");
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    if (!u || !u->conectado) {
        int err = -1;
        send(cliente_sd, &err, sizeof(err), 0);
        return NULL;
    }

    // Aquí el cliente C debe iniciar la conexión con el otro cliente usando u->ip y u->puerto.
    // Solo se devuelve 0 si todo está bien y el servidor se lo permite.
    int ok = 0;
    send(cliente_sd, &ok, sizeof(ok), 0);
    return NULL;
}


// Prepara el reenvío de un fichero (estructura base)


// Marca al usuario como desconectado
void *disconnect_user(int cliente_sd) {
    char nombre[256];
    uint8_t resultado = 3;
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo nombre en DISCONNECT");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    log_operation("DISCONNECT", nombre);
    Usuario *u = buscar_usuario(nombre);
    if (!u) {
        resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
    }
    if (!u->conectado){
        resultado = 2;
        send(cliente_sd, &resultado, 1, 0);
    }

    u->conectado = 0;
    u->ip[0] = '\0';
    u->puerto = -1;
    
    resultado = 0;
    send(cliente_sd, &resultado, 1, 0);
    return NULL;
}

// Elimina al usuario completamente del sistema
void *unregister_user(int cliente_sd) {
    uint8_t resultado = 2;
    char nombre[256];
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo nombre en UNREGISTER");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    log_operation("UNREGISTER", nombre);
    int pos = -1; // que empiece en una no real 

    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i].nombre, nombre) == 0) {
            pos = i;
            break;
        }
    }

    if ((pos = -1)){
        resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    free(usuarios[pos].ficheros);
    if (pos != num_usuarios - 1){
        usuarios[pos] = usuarios[num_usuarios - 1];
    }
    num_usuarios--;
    resultado = 0;
    send(cliente_sd, &resultado, 1, 0);
    return NULL;   
}


// Elimina un fichero publicado por un usuario
void *delete_file(int cliente_sd) {
    char nombre[256];
    char path[256];
    uint8_t resultado = 4;


    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1 ||
        readLine(cliente_sd, path, sizeof(path)) < 1) {
        perror("Error leyendo datos en DELETE");
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    log_operation("DELETE", nombre);

    Usuario *u = buscar_usuario(nombre);
    if (!u) {
        int resultado = 1;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }

    if (!u->conectado) {
        int resultado = 2;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }
    int pos = -1;
    for (int i = 0; i < u->num_ficheros; i++) {
        if (strcmp(u->ficheros[i].path, path) == 0) {
            pos = i;
            break;
        }
    }

    if ((pos = -1)){
        resultado = 3;
        send(cliente_sd, &resultado, 1, 0);
        return NULL;
    }


    if (pos != u->num_ficheros - 1){
        u->ficheros[pos] = u->ficheros[u->num_ficheros - 1 ];
    }
    u->num_ficheros--;

    if (u->num_ficheros == 0){
        free(u->ficheros);
        u->ficheros = NULL;
    } else {
        File *tmp = realloc(u->ficheros, u->num_ficheros * sizeof(File));
        if (tmp){
            u->ficheros = tmp;
        }
    }

    resultado = 0;
    send(cliente_sd, &resultado, 1, 0);
    return NULL;
}

// funcion para obtener la ip_local
void obtener_ip_local(char *ip_local, size_t size) {
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            // Verificar que la interfaz esté activa (UP) y no sea loopback
            if ((ifa->ifa_flags & IFF_UP) && !(ifa->ifa_flags & IFF_LOOPBACK)) {
                inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip_local, size);
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
}
// impresion del log al utilizar print hay que tener cuidado por lo que usamos un mutex

void log_operation(const char *op, const char *user){

    pthread_mutex_lock(&log_mutex);
    printf("s> OPERATION %s FROM %s\ns> ", op, user);
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}