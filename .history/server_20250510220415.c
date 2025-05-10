#include "server_utils.h"

extern Usuario *usuarios;
extern int num_usuarios;



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
    printf("s> %s:%d\n", ip_local, puerto); // mensaje de inicio
    fflush(stdout);
    printf("s>");

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
    printf("DEBUG: cliente conectado\n"); 

    int cliente_sd = *(int *)arg;
    free(arg);

    char comando[256];
    if (readLine(cliente_sd, comando, sizeof(comando)) < 1) {
        perror("Error de lectura ");
        close(cliente_sd);
        return NULL;
    }

    printf("DEBUG: recibido comando '%s'\n", comando);

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
    sendMessage(cliente_sd, "Comando procesado correctamente");

    close(cliente_sd);
    return NULL;
}

void *register_client(int cliente_sd) {
    char nombre[256];
    printf("DEBUG: esperando nombre del usuario...\n");
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        return NULL;
    }
    printf("DEBUG: nombre recibido: '%s'\n", nombre);
    int resultado = registrar_usuario(nombre);
    send(cliente_sd, &resultado, sizeof(resultado), 0);
    if (resultado == 0) {
        sendMessage(cliente_sd, "Usuario registrado");
        sendMessage(cliente_sd, "Comando procesado correctamente");
    } else if (resultado == 1) {
        sendMessage(cliente_sd, "ERROR: Usuario ya registrado");
    } else {
        sendMessage(cliente_sd, "ERROR: No se pudo registrar el usuario");
    }
    return NULL;
}

void *publish(int cliente_sd) {
    char nombre[256];
    char path[256];
    char descripcion[256];
    int resultado;

    printf("DEBUG: esperando nombre del usuario...\n");

    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    if (u == NULL) {
        resultado = 1;
        send(cliente_sd, &resultado, sizeof(resultado), 0);
        return NULL;
    }

    if (readLine(cliente_sd, path, sizeof(path)) < 1) {
        perror("Error leyendo el path");
        return NULL;
    }

    if (readLine(cliente_sd, descripcion, sizeof(descripcion)) < 1) {
        perror("Error leyendo la descripción");
        return NULL;
    }

    // creamos el nuevo fichero
    File *nuevo = malloc((u->num_ficheros + 1) * sizeof(File));
    if (nuevo == NULL) {
        resultado = 2;
        send(cliente_sd, &resultado, sizeof(resultado), 0);
        return NULL;
    }

    memcpy(nuevo, u->ficheros, u->num_ficheros * sizeof(File));
    free(u->ficheros);
    u->ficheros = nuevo;
    strcpy(u->ficheros[u->num_ficheros].path, path);
    strcpy(u->ficheros[u->num_ficheros].descripcion, descripcion);
    u->num_ficheros++;

    printf("DEBUG: fichero publicado: path='%s', descripcion='%s'\n",
        u->ficheros[u->num_ficheros - 1].path,
        u->ficheros[u->num_ficheros - 1].descripcion);

    resultado = 0;
    send(cliente_sd, &resultado, sizeof(resultado), 0); // seguimos usando send() para enteros
    return NULL;
}
/*
	•	Leer nombre
	•	Leer puerto
	•	Buscar usuario
	•	Validar si está o no conectado
    •   Obtener ip
*/

void *connect_user(int cliente_sd) {
    char nombre[256];
    int puerto;
    int resultado;

    printf("DEBUG: esperando nombre del usuario...\n");
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        return NULL;
    }
    printf("DEBUG: nombre recibido: '%s'\n", nombre);

    // recv espera un int
    if (recv(cliente_sd, &puerto, sizeof(puerto), 0) <= 0) {
        perror("Error leyendo el puerto del usuario");
        return NULL;
    }
    printf("DEBUG: puerto recibido: %d\n", puerto);

    Usuario *u = buscar_usuario(nombre);
    if (u == NULL) {
        resultado = -1;
        send(cliente_sd, &resultado, sizeof(resultado), 0);
        return NULL;
    }

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getpeername(cliente_sd, (struct sockaddr *)&addr, &addrlen) != 0) {
        perror("Error obteniendo IP del cliente");
        resultado = -2;
        send(cliente_sd, &resultado, sizeof(resultado), 0);
        return NULL;
    }
    // Convierte la dirección IP binaria del cliente a formato de texto y la guarda en u->ip
    inet_ntop(AF_INET, &addr.sin_addr, u->ip, sizeof(u->ip));
    u->puerto = puerto;
    u->conectado = 1;

    resultado = 0;
    send(cliente_sd, &resultado, sizeof(resultado), 0);
    return NULL;
}


void *list_users(int cliente_sd) {
    char linea[512];
    for (int i = 0; i < num_usuarios; i++) {
        if (usuarios[i].conectado) {
            snprintf(linea, sizeof(linea), "%s %s %d\n",
                     usuarios[i].nombre,
                     usuarios[i].ip,
                     usuarios[i].puerto);
            sendMessage(cliente_sd, linea);
        }
    }
    // Enviar línea vacía para marcar el final
    sendMessage(cliente_sd, "\n");
    return NULL;
}
/*
typedef struct {
    char path[256];
    char descripcion[256];
} File; //fichero publicado

typedef struct {
    char nombre[256];
    int conectado;
    char ip[64];
    int puerto;
    File *ficheros;
    int num_ficheros;
} Usuario; //user registrado
*/

// Devuelve la lista de ficheros publicados por un usuario conectado
void *list_content(int cliente_sd) {
    char nombre[256];
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo nombre en LIST_CONTENT");
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    if (!u || !u->conectado) {
        sendMessage(cliente_sd, "\n");
        return NULL;
    }

    for (int i = 0; i < u->num_ficheros; i++) {
        char linea[512];
        snprintf(linea, sizeof(linea), "%s %s\n", u->ficheros[i].path, u->ficheros[i].descripcion);
        sendMessage(cliente_sd, linea);
    }
    sendMessage(cliente_sd, "\n");
    return NULL;
}

// Prepara el reenvío de un fichero (estructura base)
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

// Marca al usuario como desconectado
void *disconnect_user(int cliente_sd) {
    char nombre[256];
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo nombre en DISCONNECT");
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    if (u) {
        u->conectado = 0;
        u->ip[0] = '\0';
        u->puerto = -1;
    }

    int ok = 0;
    send(cliente_sd, &ok, sizeof(ok), 0);
    return NULL;
}

// Elimina al usuario completamente del sistema
void *unregister_user(int cliente_sd) {
    char nombre[256];
    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo nombre en UNREGISTER");
        return NULL;
    }

    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i].nombre, nombre) == 0) {
            free(usuarios[i].ficheros);
            for (int j = i; j < num_usuarios - 1; j++) {
                usuarios[j] = usuarios[j + 1];
            }
            num_usuarios--;
            int ok = 0;
            send(cliente_sd, &ok, sizeof(ok), 0);
            return NULL;
        }
    }

    int error = -1;
    send(cliente_sd, &error, sizeof(error), 0);
    return NULL;
}

// Elimina un fichero publicado por un usuario
void *delete_file(int cliente_sd) {
    char nombre[256];
    char path[256];

    if (readLine(cliente_sd, nombre, sizeof(nombre)) < 1 ||
        readLine(cliente_sd, path, sizeof(path)) < 1) {
        perror("Error leyendo datos en DELETE");
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    if (!u) {
        int err = -1;
        send(cliente_sd, &err, sizeof(err), 0);
        return NULL;
    }

    int found = 0;
    for (int i = 0; i < u->num_ficheros; i++) {
        if (strcmp(u->ficheros[i].path, path) == 0) {
            for (int j = i; j < u->num_ficheros - 1; j++) {
                u->ficheros[j] = u->ficheros[j + 1];
            }
            u->num_ficheros--;
            found = 1;
            break;
        }
    }

    int result = found ? 0 : -1;
    send(cliente_sd, &result, sizeof(result), 0);
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
            inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, ip_local, size);
            break;
        }
    }

    freeifaddrs(ifaddr);
}