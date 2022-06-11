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
#include <ncurses.h>


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
          exit(EXIT_FAILURE);
        }
      }
    }
  }

  freeifaddrs(ifaddr);

  return inet_addr( host );
}

typedef struct {
    char* ch;
    u_int8_t* work_flag;
    int port;
    char* ip_addr;
} pthrData;

void* thread_func(void* thread_data) {
    pthrData *data = (pthrData*) thread_data;

    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(data->port);
    servaddr.sin_addr.s_addr = inet_addr(data->ip_addr);

    while(*(data->work_flag) == 1) {
       *(data->ch) = getch();

        sendto(sockfd, data->ch, 1,
                MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                sizeof(servaddr));
    }
    close(sockfd);
}

void server(int remote_port, char* remote_ip, char* ch, uint8_t* work_flag) {
    int sockfd;
    char buff;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(remote_port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(cliaddr);
    int recieve = recvfrom(sockfd, ch, 1,
                    MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);

    u_int played_ip = cliaddr.sin_addr.s_addr;

    while (recieve >= 1)
    {
        recieve = recvfrom(sockfd, &buff, 1,
                    MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);

        if (cliaddr.sin_addr.s_addr == played_ip)
            *ch = buff;

        // printf("%c\n", *ch);

         if (*(ch) == '2')
        {
           //printf("recieved 2\n");
            *(work_flag) = 2;
            break;
        }
        if (*(ch) == '3')
        {
        //printf("recieved 3\n");
            *(work_flag) = 3;
            break;
        }
        if (*(ch) == '4')
        {
        //printf("recieved 4\n");
            *(work_flag) = 4;
            break;
        }
    }
}

int controller_server(int remote_port, char* remote_ip, char* ch1, char* ch2, uint8_t* work_flag) {
    u_int local_ip_number = ntohl(get_local_ip());
    u_int remote_ip_number = ntohl(inet_addr(remote_ip));

    pthread_t* thread_1 = malloc(sizeof(pthread_t));

    // printf("%lu %lu\n", remote_ip_number, local_ip_number);

    if (local_ip_number > remote_ip_number) {
        pthrData thread_1_data = {
            .ch = ch1,
            .work_flag = work_flag,
            .port = remote_port,
            .ip_addr = remote_ip
        };
        pthread_create(thread_1, NULL, thread_func, &thread_1_data);
        server(remote_port, remote_ip, ch2, work_flag);
    }
    else {
       pthrData thread_1_data = {
            .ch = ch2,
            .work_flag = work_flag,
            .port = remote_port,
            .ip_addr = remote_ip
        };
        pthread_create(thread_1, NULL, thread_func, &thread_1_data);
        server(remote_port, remote_ip, ch1, work_flag);
    }

    pthread_join(*thread_1, NULL);

    free(thread_1);

    return 0;
}
