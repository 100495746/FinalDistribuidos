#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char path[256];
    char descripcion[256];
} File; //fichero publicado

typedef struct {
    char nombre[256];
    int conectado; //0 =
    char ip[64];
    int puerto;
    File *ficheros;
    int num_ficheros;
} Usuario; //user registrado

//  prototipos
int registrar_usuario(const char *nombre);
void imprimir_usuarios();
Usuario *buscar_usuario(const char *nombre);

#endif