#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H
#define _GNU_SOURCE   // sin esta cabecera parece ser que no lee las variables "IFF_" que se encuentran en net_if.h
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>  // Para inet_ntop()
#include <net/if.h>                          
#include <ifaddrs.h>




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


// funcion para obtener la ip_local
void obtener_ip_local(char *ip_local, size_t size);

// funcion para los logs de cada usuario en el servidor
void log_operation(const char *operation, const char *user);

//  prototipos
int registrar_usuario(const char *nombre);
int existe_usuario(const char *nombre);
void imprimir_usuarios();
Usuario *buscar_usuario(const char *nombre);

void *register_client(int cliente_sd);
void *publish(int cliente_sd);
void *connect_user(int cliente_sd);
void *list_users(int cliente_sd);
void *list_content(int cliente_sd);
void *get_file(int cliente_sd);
void *disconnect_user(int cliente_sd);
void *unregister_user(int cliente_sd);
void *delete_file(int cliente_sd);
void *handle_client(void *arg);


int sendMessage(int fd, const char *msg);
int readLine(int fd, void *buffer, size_t n);

#endif