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
    int conectado; //0 = no conectado
    char ip[64];
    int puerto;
    File *ficheros;
    int num_ficheros;
} Usuario; //user registrado

//  prototipos
int registrar_usuario(const char *nombre);
int existe_usuario(const char *nombre);
void imprimir_usuarios();
Usuario *buscar_usuario(const char *nombre);
void *register_client(int cliente_sd);
void *publish(int cliente_sd);
void *connect_user(int cliente_sd);
int sendMessage(int fd, const char *msg);
int readLine(int fd, void *buffer, size_t n);

#endif