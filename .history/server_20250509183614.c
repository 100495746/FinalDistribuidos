#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "server_utils.h"
#include <arpa/inet.h>  // Para inet_ntop()


void *handle_client(void *arg);
int main() {
    int puerto = 5050; // Define el puerto a usar
    int sd = socket(AF_INET, SOCK_STREAM, 0); //usa IPv4 y por tcp (un socket es parecido a un descriptor de archivo fd)

    struct sockaddr_in direccion;
    direccion.sin_family = AF_INET;          // Tipo de dirección: IPv4
    direccion.sin_port = htons(puerto);      // Puerto del servidor (en orden de red) host to network
    direccion.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones desde cualquier IP

    bind(sd, (struct sockaddr *)&direccion, sizeof(direccion)); //asociamos el socket a la ip y puerto local

    listen(sd,10); // "encendemos" el socket para recibir conexiones externas, ponemos 10 como maximo de cola
    printf("s> init server 0.0.0.0:%d\n", puerto); // mensaje de inicio
    printf("s>\n");

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
    if (read(cliente_sd, comando, 256) < 1) {
        perror("Error de lectura ");
        close(cliente_sd);
        return NULL;
    }

    printf("DEBUG: recibido comando '%s'\n", comando);

    if (strcmp(comando, "REGISTER") == 0) {
        register_client(cliente_sd);
    } else if (strcmp(comando, "PUBLISH") == 0) {
        publish(cliente_sd);
    } else if (strcmp(comando, "CONNECT") == 0) {
        connect_user(cliente_sd);
    }

    close(cliente_sd);
    return NULL;
}

void *register_client(int cliente_sd) {
    char nombre[256];
    printf("DEBUG: esperando nombre del usuario...\n");

    int i = 0;
    char c;
    ssize_t n;
    while (i < 255) {
        n = read(cliente_sd, &c, 1);
        if (n <= 0) {
            perror("Error leyendo carácter del nombre");
            nombre[0] = '\0';  // para evitar basura
            break;
        }
        nombre[i++] = c;
        if (c == '\0') break;
    }da 
    nombre[i] = '\0';

    printf("DEBUG: nombre recibido: '%s'\n", nombre);
    int resultado = registrar_usuario(nombre);
    write(cliente_sd, &resultado, sizeof(resultado));
    return NULL;
}

void *publish(int cliente_sd) {
    char nombre[256];
    char path[256];
    char descripcion[256];
    int resultado;

    if (read(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        return NULL;
    }
    printf("DEBUG: esperando nombre del usuario...\n");

    Usuario *u = buscar_usuario(nombre);
    if (u == NULL) {
        resultado = 1;
        write(cliente_sd, &resultado, sizeof(resultado));
        return NULL;
    }

    if (read(cliente_sd, path, sizeof(path)) < 1) {
        perror("Error leyendo el path");
        return NULL;
    }

    if (read(cliente_sd, descripcion, sizeof(descripcion)) < 1) {
        perror("Error leyendo la descripción");
        return NULL;
    }

    File *nuevo = malloc((u->num_ficheros + 1) * sizeof(File));
    if (nuevo == NULL) {
        resultado = 2;
        write(cliente_sd, &resultado, sizeof(resultado));
        return NULL;
    }

    memcpy(nuevo, u->ficheros, u->num_ficheros * sizeof(File));
    free(u->ficheros);
    u->ficheros = nuevo;
    strcpy(u->ficheros[u->num_ficheros].path, path);
    strcpy(u->ficheros[u->num_ficheros].descripcion, descripcion);
    u->num_ficheros++;

    resultado = 0;
    write(cliente_sd, &resultado, sizeof(resultado));
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

    if (read(cliente_sd, nombre, sizeof(nombre)) < 1) {
        perror("Error leyendo el nombre del usuario");
        return NULL;
    }
    printf("DEBUG: esperando nombre del usuario...\n");
    if (read(cliente_sd, &puerto, sizeof(puerto)) < 1) {
        perror("Error leyendo el puerto del usuario");
        return NULL;
    }

    Usuario *u = buscar_usuario(nombre);
    if (u == NULL) {
        resultado = -1;
        write(cliente_sd, &resultado, sizeof(resultado));
        return NULL;
    }

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    if (getpeername(cliente_sd, (struct sockaddr *)&addr, &addrlen) != 0) {
        perror("Error obteniendo IP del cliente");
        resultado = -2;
        write(cliente_sd, &resultado, sizeof(resultado));
        return NULL;
    }

    inet_ntop(AF_INET, &addr.sin_addr, u->ip, sizeof(u->ip));
    u->puerto = puerto;
    u->conectado = 1;

    resultado = 0;
    write(cliente_sd, &resultado, sizeof(resultado));
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