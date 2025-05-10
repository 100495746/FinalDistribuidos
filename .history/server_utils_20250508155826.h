#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

typedef struct {
    char path[256];
    char descripcion[256];
} File;

typedef struct {
    char nombre[256];
    int conectado;
    char ip[64];
    int puerto;
    File *ficheros;
    int num_ficheros;
} Usuario;

// Aquí pondrás prototipos como:
int registrar_usuario(const char *nombre);
void imprimir_usuarios();

#endif