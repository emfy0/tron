#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


int main(int argc, char* argv[]) {
    int sd;
    int cd;
    struct sockaddr_in addr, remoteaddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if( sd < 0 ) {
        perror("Error calling socket");
        close(sd);
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2021);
    // addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    addr.sin_addr.s_addr = INADDR_ANY;

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

    socklen_t remoteaddr_len;
    if ( 0 > (cd = accept(sd, (struct sockaddr *)&remoteaddr, &remoteaddr_len ))) {
        perror("Accept");
        close(sd);
        close(cd);
        return 4;
    }

    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(remoteaddr.sin_addr), str, INET_ADDRSTRLEN);
    printf("%d %s\n", remoteaddr.sin_addr.s_addr, str);
    char ch;
    while(1) {


        recv(cd, &ch, 1, 0);
        if (ch == 'q')
            break;
    }
    close(cd);
    close(sd);
    return 0;
}
