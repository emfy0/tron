#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <stdint.h>

typedef struct {
    int remote_port; char* remote_ip; char* ch1; char* ch2; uint8_t* work_flag;
} Server_data;

int controller_server(int remote_port, char *remote_ip, char* ch1, char* ch2, uint8_t* work_flag);

#endif
