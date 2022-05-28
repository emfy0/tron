#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <stdint.h>

typedef struct {
    int local_port; int remote_port; char* remote_ip; char* ch1; char* ch2; uint8_t* work_flag;
} Server_data;

int controller_server(int local_port, int remote_port, char *remote_ip, char* ch1, char* ch2, uint8_t* work_flag);

typedef struct {
    char* remote_ip; int local_port; int remote_port; uint8_t* work_flag;
} Client_data;

int controller_client(char* remote_ip, int local_port, int remote_port, uint8_t* work_flag);

#endif
