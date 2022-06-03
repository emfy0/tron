#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ncurses.h>

#include "controller.h"

int connect_to(char *ip_addr, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip_addr);

    int cd = socket(AF_INET, SOCK_STREAM, 0);
    if ( cd == -1) {
        close(cd);
        return -1;
    }

    if (connect(cd,(struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(cd);
        return -2;
    }

    return cd;
}

int controller_client(char* remote_ip, int local_port, int remote_port, uint8_t* work_flag) {
    int local_cd = connect_to("127.0.0.1", local_port);
    int remote_cd = connect_to(remote_ip, remote_port);

    {
        int counter = 0;
        while (counter < 10) {
            if (remote_cd > 0) {
                break;
            }
            remote_cd = connect_to(remote_ip, remote_port);
            counter++;
            refresh();
            sleep ( 1);
        }
    }
    clear();

    if ( local_cd <= 0 || remote_cd <= 0 ) { return -1; *work_flag = 0; }

    char ch;
    while(ch != 'p' && *work_flag == 1) {
        ch = getch();
        for (int i = 0; i < 10; i++) {
            if (*work_flag == 2) {
                char char_asd = '2';
                send(local_cd, &char_asd, 1, 0);
                send(remote_cd, &char_asd, 1, 0);
                break;
            }
            if (*work_flag == 3) {
                char char_asd = '3';
                send(local_cd, &char_asd, 1, 0);
                send(remote_cd, &char_asd, 1, 0);
                break;
            }
            if (*work_flag == 4) {
                char char_asd = '4';
                send(local_cd, &char_asd, 1, 0);
                send(remote_cd, &char_asd, 1, 0);
                break;
            }
        }
        send(local_cd, &ch, 1, 0);
        send(remote_cd, &ch, 1, 0);

    }
    *work_flag = 0;

    close(remote_cd);
    close(local_cd);
    return 0;
}
