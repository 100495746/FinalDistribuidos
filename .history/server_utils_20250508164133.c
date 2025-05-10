#include "server_utils.h"
#include <stdlib.h>
#include <string.h>
Usuario *usrpt = NULL; //pointer a structs de usuarios ()
int num_usuarios = 0;
int capacidad_usuarios = 0;

//struct Usuario *usrpt = (Usuario *)malloc(sizeof(Usuario));

int registrar_usuario(const char *nombre) {
    // Comprobar si ya existe el usuario
    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usrpt[i].nombre, nombre) == 0) {
            return 1; // USERNAME IN USE
        }
    }

    // Reservar espacio para un nuevo usuario usando crecimiento dinámico eficiente
    Usuario *nuevo = malloc((num_usuarios+1) * sizeof(Usuario));
    if (nuevo == NULL) {
        return 2; // Error de memoria
    }
    // Copiar los usuarios existentes al nuevo bloque
    if (usrpt != NULL) {
        memcpy(nuevo, usrpt, num_usuarios * sizeof(Usuario));
        free(usrpt);
    }
    usrpt = nuevo;

    // Inicializar nuevo usuario
    strncpy(usrpt[num_usuarios].nombre, nombre, sizeof(usrpt[num_usuarios].nombre) - 1);
    usrpt[num_usuarios].nombre[sizeof(usrpt[num_usuarios].nombre) - 1] = '\0';
    usrpt[num_usuarios].conectado = 0;
    usrpt[num_usuarios].puerto = -1;
    usrpt[num_usuarios].ip[0] = '\0';
    usrpt[num_usuarios].ficheros = NULL;
    usrpt[num_usuarios].num_ficheros = 0;

    num_usuarios++;
    return 0; // OK
}

int existe_usuario(const char *nombre){

}

Usuario *buscar_usuario(const char *nombre){

}

