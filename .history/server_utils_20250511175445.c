#include "server_utils.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

Usuario *usuarios = NULL; //pointer a structs de usuarios ()
int num_usuarios = 0;
int capacidad_usuarios = 0;

//struct Usuario *usrpt = (Usuario *)malloc(sizeof(Usuario));

int registrar_usuario(const char *nombre) {
    // Comprobar si ya existe el usuario
    if(existe_usuario(nombre)==1){
        return 1;
    }

    // Reservar espacio para un nuevo usuario usando crecimiento dinámico eficiente
    Usuario *nuevo = malloc((num_usuarios+1) * sizeof(Usuario));
    if (nuevo == NULL) {
        return 2; // Error de memoria
    }
    // Copiar los usuarios existentes al nuevo bloque
    if (usuarios != NULL) {
        memcpy(nuevo, usuarios, num_usuarios * sizeof(Usuario));
        free(usuarios);
    }
    usuarios = nuevo;

    // Inicializar nuevo usuario
    strncpy(usuarios[num_usuarios].nombre, nombre, sizeof(usuarios[num_usuarios].nombre) - 1);
    usuarios[num_usuarios].nombre[sizeof(usuarios[num_usuarios].nombre) - 1] = '\0'; //aseguro terminación
    usuarios[num_usuarios].conectado = 0; // no conectado al ppio
    usuarios[num_usuarios].puerto = -1; //puerto inexistente al empezar ( no conectado aun)
    usuarios[num_usuarios].ip[0] = '\0'; // ip vacío hasta que se conecte
    usuarios[num_usuarios].ficheros = NULL;  
    usuarios[num_usuarios].num_ficheros = 0;

    num_usuarios++;
    return 0; // OK
}

int existe_usuario(const char *nombre){
    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i].nombre, nombre) == 0) {
            return 1; // USERNAME IN USE
        }
    }
    return 0;
}

Usuario *buscar_usuario(const char *nombre){
    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i].nombre, nombre) == 0) {
            return &usuarios[i]; //devuelve lo encontrado
        }
    }
    return NULL;
}

int readLine(int fd, void *buffer, size_t n) {
    char *buf = buffer;
    size_t i = 0;
    char c;
    while (i < n - 1) {
        ssize_t bytes = read(fd, &c, 1);
        if (bytes <= 0) return -1; // error o conexión cerrada
        buf[i++] = c;
        if (c == '\0') break;
    }
    buf[i] = '\0';
    return i;
}


int sendMessage(int fd, const char *msg) {
    size_t len = strlen(msg) + 1;
    return send(fd, msg, len, 0);
}

