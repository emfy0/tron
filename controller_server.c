#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> //errno
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h> //memset
#include <sys/socket.h>


#include "controller.h"


u_int get_local_ip() {
    FILE *f;
    char line[100], *p, *c;

    f = fopen("/proc/net/route", "r");

    while (fgets(line, 100, f)) {
        p = strtok(line, " \t");
        c = strtok(NULL, " \t");

        if (p != NULL && c != NULL) {
            if (strcmp(c, "00000000") == 0) {
                // printf("Default interface is : %s \n", p);
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
        //   printf("getnameinfo() failed: %s\n", gai_strerror(s));
          exit(EXIT_FAILURE);
        }

        // printf("address: %s", host);
      }
    //   printf("\n");
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
    if (ip_addr == NULL) {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else {
        addr.sin_addr.s_addr = inet_addr( ip_addr );
    }

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

typedef struct {
    char* ch;
    int cd;
    u_int8_t* work_flag;
} pthrData;

void* thread_func(void* thread_data) {
     pthrData *data = (pthrData*) thread_data;

    while(*(data->work_flag)) {
        int recieve = recv(data->cd, data->ch, 1, 0);
        // printf("%c\n", *(data->ch));
        if (*(data->ch) == 'p' || recieve == -1 || recieve == 0)
            *(data->work_flag) = 0;
    }

    return NULL;
}

int controller_server(int local_port, int remote_port, char* remote_ip, char* ch1, char* ch2, uint8_t* work_flag) {
    char local_ip[] = "127.0.0.1";

    int local_sd = sd_listen_to(local_ip, local_port);
    if (local_sd < 0) {
        perror("error making local_sd:");
        return -1;
    }

    // printf("waiting for local connection\n");
    int local_cd = accept(local_sd, NULL, NULL );
    if ( 0 > local_cd ) {
        perror("Accept");
        close(local_sd);
        close(local_cd);
        return -1;
    }
    u_int local_ip_number = ntohl(get_local_ip());
    // printf("local_ip_number: %lu\n", local_ip_number);

    int remote_sd = sd_listen_to(NULL, remote_port);
    if (remote_sd < 0) {
        perror("error making remote_sd:");
        return -1;
    }

    struct sockaddr_in* remoteaddr = malloc(sizeof(struct sockaddr_in));
    socklen_t remoteaddr_len;
    // printf("waiting for remote connection\n");
    int remote_cd = accept(remote_sd, (struct sockaddr*)remoteaddr, &remoteaddr_len );
    if ( 0 > remote_cd ) {
        perror("Accept");
        close(remote_sd);
        close(remote_cd);
        return -1;
    }
    u_int remote_ip_number = ntohl(inet_addr(remote_ip));
    free(remoteaddr);

    pthread_t* thread_1 = malloc(sizeof(pthread_t));
    pthread_t* thread_2 = malloc(sizeof(pthread_t));

    // printf("%lu %lu\n", remote_ip_number, local_ip_number);

    pthrData thread_1_data, thread_2_data;
    thread_1_data.cd = local_cd;
    thread_2_data.cd = remote_cd;
    thread_1_data.work_flag = work_flag;
    thread_2_data.work_flag = work_flag;

    if (local_ip_number > remote_ip_number) {
        thread_1_data.ch = ch1;
        thread_2_data.ch = ch2;
    }
    else {
        thread_1_data.ch = ch2;
        thread_2_data.ch = ch1;
    }

    pthread_create(thread_1, NULL, thread_func, &thread_1_data);
    pthread_create(thread_2, NULL, thread_func, &thread_2_data);

    pthread_join(*thread_1, NULL);
    pthread_join(*thread_2, NULL);

    // printf("ch1: '%c' ch2: '%c'", *ch1, *ch2);

    free(thread_1);
    free(thread_2);

    close(local_cd);
    close(remote_cd);
    close(local_sd);
    close(remote_sd);
    return 0;
}
