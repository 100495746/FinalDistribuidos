#include "rpc_logger.h"
#include <stdio.h>

void *log_event_1_svc(char **msg, struct svc_req *req) {
    printf("📘 RPC log recibido: %s\n", *msg);
    fflush(stdout);
    return NULL;
}