

#include <sys/socket.h>
#include <netinet/in.h>

int sd;

int main() {
    int puerto = 5000; // Define el puerto a usar
    sd = socket(AF_INET, SOCK_STREAM, 0); //usa IPv4 y por tcp
    struct sockaddr_in direccion;
    direccion.sin_family = AF_INET;          // Tipo de dirección: IPv4
    direccion.sin_port = htons(puerto);      // Puerto del servidor (en orden de red) host to network
    direccion.sin_addr.s_addr = INADDR_ANY;  // Aceptar conexiones desde cualquier IP

    return 0;
}