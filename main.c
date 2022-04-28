#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

#include "controller.h"

void *thread_server (void* thread_data) {
    Server_data* data = (Server_data*)thread_data;
    controller_server(data->local_port, data->remote_port, data->ch1, data->ch2, data->work_flag);
    return NULL;
}

void *thread_client (void* thread_data) {
    Client_data* data = (Client_data*)thread_data;
    controller_client(data->remote_ip, data->local_port, data->remote_port, data->work_flag);
    return NULL;
}

int main() {
    char* remote_ip = "192.168.1.38";

    pthread_t* thread_1 = malloc(sizeof(pthread_t));
    pthread_t* thread_2 = malloc(sizeof(pthread_t));
    uint8_t work_flag = 1;
    char ch1, ch2;

    Server_data server_data = {
        .local_port = 2021,
        .remote_port = 12345,
        .ch1 = &ch1,
        .ch2 = &ch2,
        .work_flag = &work_flag
    };

    Client_data client_data= {
        .remote_ip = remote_ip,
        .local_port = 2021,
        .remote_port = 12345,
        .work_flag = &work_flag
    };

    initscr();
    noecho();

    pthread_create(thread_1, NULL, thread_server, &server_data);
    pthread_create(thread_2, NULL, thread_client, &client_data);

    pthread_join(*thread_1, NULL);
    pthread_join(*thread_2, NULL);

    free(thread_1);
    free(thread_2);

    endwin();

    printf("MAIN ch1: '%c' ch2: '%c'", ch1, ch2);
}
