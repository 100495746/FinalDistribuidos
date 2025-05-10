#include "server_utils.h"
#include <stdlib.h>
#include <string.h>

struct Usuario *usrpt = NULL; //pointer a structs de usuarios ()
int num_usuarios = 0;

int registrar_usuario(const char *nombre){

int registrar_usuario(const char *nombre){
	if (usrpt == NULL) {
		usrpt = (Usuario *)malloc(sizeof(Usuario));
		if (usrpt == NULL) {
			return -1; // Error allocating memory
		}
	}
	// TODO: Add logic to register the user with the given name
	return 0;
}

}

Usuario *buscar_usuario(const char *nombre){

}

