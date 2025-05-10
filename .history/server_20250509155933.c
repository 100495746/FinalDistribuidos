#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

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
    if (strcmp(comando, "PUBLISH") == 0) {
        char nombre[256]; 
        //leemos lo siguiente, el registro de usuario necesita el nombre
        if (read(cliente_sd, nombre, sizeof(nombre)) < 1) {
            perror("Error leyendo el nombre del usuario");
            close(cliente_sd);
            return NULL;
        }
        
        
        
        
        // Lo enviamos al cliente
        write(cliente_sd, &resultado, sizeof(resultado));
}


        close(cliente_sd); // cerrar el socket al final
        return NULL;
    }