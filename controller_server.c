#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int sd;
int main(int argc, char* argv[]) {
    int cd;
    struct sockaddr_in addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if( sd < 0 ) {
        perror("Error calling socket");
        close(sd);
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2021);
    addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

    if( bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
        perror("Bind");
        close(sd);
        return 2;
    }
    
    if( listen(sd, 5) ) {
        perror("Listen");
        close(sd);
        return 3;
    }

    if ( 0 > (cd = accept(sd, NULL, NULL)) ) {
        perror("Accept");
        close(sd);
        close(cd);
        return 4;
    }
    
    char ch;
    while(1) {


        recv(cd, &ch, 1, 0);
        printf("%c\n", ch);
        if (ch == 'q')
            break;
    }
    close(cd);
    return 0;
}

void __attribute__((destructor)) destructor(void) {
    close(sd);
}
