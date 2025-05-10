

#include <sys/socket.h>
#include <netinet/in.h>

int sd;

int main() {
	sd = socket(AF_INET, SOCK_STREAM, 0); //usa IPv4 y por tcp
    struct sockaddr_in direccion;
    

	return 0;
}