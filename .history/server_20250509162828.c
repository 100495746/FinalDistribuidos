#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "server_utils.h"

void *handle_client(void *arg);
int main() {
    int puerto = 5000; // Define el puerto a usar
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
    int cliente_sd = *(int *)arg;
    free(arg);
    
    char comando[256];
    if(read(cliente_sd, comando, 256)<1){
            perror("Error de lectura ");
            return -1;
    }

    // 1. Leer comando del cliente con read() (ej. "REGISTER")
    // 2. Comparar con strcmp()
    // 3. Leer el nombre del usuario (otro read())
    // 4. Llamar a registrar_usuario()
    // 5. Enviar el resultado al cliente con write()
    if (strcmp(comando, "REGISTER") == 0) {
            char nombre[256]; 
            //leemos lo siguiente, el registro de usuario necesita el nombre
            if (read(cliente_sd, nombre, sizeof(nombre)) < 1) {
                perror("Error leyendo el nombre del usuario");
                close(cliente_sd);
                return NULL;
            }
            
            // Registramos este nombre
            int resultado = registrar_usuario(nombre);
            
            // Lo enviamos al cliente
            write(cliente_sd, &resultado, sizeof(resultado));
    }

     // 1. Leer nombre del usuario
    // 2. Leer path del fichero
    // 3. Leer descripción
    // 4. Llamar a publicar_fichero(...)
    // 5. Enviar código de resultado al cliente
    if (strcmp(comando, "PUBLISH") == 0) {
        
        char nombre[256]; 
        int conectado;
        char ip[64]; 
        char path[256];
        
        //leemos lo siguiente, vamos a pasar el read por todos hasta el path

        if (read(cliente_sd, nombre, sizeof(nombre)) < 1) {
            perror("Error leyendo el nombre del usuario");
            close(cliente_sd);
            return NULL;
        }

        //buscamos este usuario
        Usuario *u = buscar_usuario(nombre);



        if (read(cliente_sd, conectado, sizeof(int)) < 1) {
            perror("Error leyendo el estado de conexión del usuario");
            close(cliente_sd);
            return NULL;
        }

        if (read(cliente_sd, ip, sizeof(path)) < 1) {
            perror("Error leyendo el estado de conexión del usuario");
            close(cliente_sd);
            return NULL;
        }


        File *nuevo = malloc((u->num_ficheros+1) * sizeof(File));
        if (nuevo == NULL) {
            return 2; // Error de memoria
        }
        // Copiar los files existentes al nuevo array
        if (u != NULL) {
            memcpy(nuevo, u->ficheros, u->num_ficheros * sizeof(File));
            free(u);
        }
        u = nuevo;

        if



        /*
        typedef struct {
    char nombre[256];
    int conectado;
    char ip[64];
    int puerto;
    File *ficheros;
    int num_ficheros;
} Usuario; //user registrado

typedef struct {
    char path[256];
    char descripcion[256];
} File; //fichero publicado
        
        
        */
        
        
        
        
        
    }


        close(cliente_sd); // cerrar el socket al final
        return NULL;
    }