#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h> //errno
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h> //memset
#include <sys/socket.h>


u_int get_local_ip() {
    FILE *f;
  char line[100], *p, *c;

  f = fopen("/proc/net/route", "r");

  while (fgets(line, 100, f)) {
    p = strtok(line, " \t");
    c = strtok(NULL, " \t");

    if (p != NULL && c != NULL) {
      if (strcmp(c, "00000000") == 0) {
        printf("Default interface is : %s \n", p);
        break;
      }
    }
  }

  // which family do we require , AF_INET or AF_INET6
  int fm = AF_INET;
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  // Walk through linked list, maintaining head pointer so we can free list
  // later
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }

    family = ifa->ifa_addr->sa_family;

    if (strcmp(ifa->ifa_name, p) == 0) {
      if (family == fm) {
        s = getnameinfo(ifa->ifa_addr,
                        (family == AF_INET) ? sizeof(struct sockaddr_in)
                                            : sizeof(struct sockaddr_in6),
                        host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if (s != 0) {
          printf("getnameinfo() failed: %s\n", gai_strerror(s));
          exit(EXIT_FAILURE);
        }

        printf("address: %s", host);
      }
      printf("\n");
    }
  }

  freeifaddrs(ifaddr);

  return inet_addr( host );
}

int sd_listen_to(char* ip_addr, int port) {
    struct sockaddr_in addr;
    int sd = socket(AF_INET, SOCK_STREAM, 0);

    if( sd < 0 ) {
        perror("Error calling socket");
        close(sd);
        return -1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr( ip_addr );

    if( bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
        perror("Bind");
        close(sd);
        return -2;
    }

    if( listen(sd, 5) ) {
        perror("Listen");
        close(sd);
        return -3;
    }

    return sd;
}

int main(int argc, char* argv[]) {
    char ip[] = "127.0.0.1";
    int port = 2021;

    int local_sd = sd_listen_to(ip, port);
    if (local_sd < 0) {
        perror("error making local_sd:");
        return -1;
    }

    int local_cd = accept(local_sd, NULL, NULL );
    if ( 0 > local_cd ) {
        perror("Accept");
        close(local_sd);
        close(local_cd);
        return -1;
    }
    u_int local_ip_number = ntohl(get_local_ip());

    int remote_sd = sd_listen_to(ip, port);
    if (remote_sd < 0) {
        perror("error making remote_sd:");
        return -1;
    }

    struct sockaddr_in remoteaddr;
    socklen_t remoteaddr_len;
        int remote_cd = accept(remote_sd, (struct sockaddr *)&remoteaddr, &remoteaddr_len );
    if ( 0 > remote_cd ) {
        perror("Accept");
        close(remote_sd);
        close(remote_cd);
        return -1;
    }
    u_int remote_ip_number = ntohl(remoteaddr.sin_addr.s_addr);


    // char ch;
    // while(1) {
    //     int recieve = recv(cd, &ch, 1, 0);
    //     printf("%c\n", ch);
    //     if (ch == 'q' || recieve == -1 || recieve == 0)
    //         break;
    // }

    close(local_cd);
    close(local_sd);
    close(remote_cd);
    close(remote_sd);
    return 0;
}
