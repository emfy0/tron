#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ncurses.h>

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

int main(int argc, char* argv[]) {
    int local_port = 2021;
    char remote_ip[] = "192.168.1.38";
    int remote_port = 12345;

    int local_cd = connect_to("127.0.0.1", local_port);
    printf( "connecting to remote ip...\n");
    int remote_cd = connect_to(remote_ip, remote_port);

    {
        int counter = 0;
        while (counter < 10) {
            if (remote_cd > 0) {
                break;
            }
            remote_cd = connect_to(remote_ip, remote_port);
            counter++;
            printf("times of retrying left: %d\n", 11 - counter);
            sleep ( 1);
        }
    }

    initscr();
    noecho();

    char ch, ch1;
    while(ch != 'p') {
        ch = getch();
        ch1 = ch + 1;
        send(local_cd, &ch, 1, 0);
        send(remote_cd, &ch1, 1, 0);

    }

    close(remote_cd);
    close(local_cd);
    return 0;
}
