#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ncurses.h>

int main(int argc, char* argv[]) {
    int cd;
    struct sockaddr_in addr;


    if ( 0 > (cd = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Error calling socket");
        close(cd);
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(2021);
    addr.sin_addr.s_addr = inet_addr("178.140.145.36");
    
    if ( (-1) == connect(cd,(struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect");
        close(cd);
        return 2;
    }

    initscr();

    char ch, buff;
    while(ch != 'p') {
        ch = getch();
        send(cd, &ch, 1, 0);
    }

    close(cd);
    return 0;
}
