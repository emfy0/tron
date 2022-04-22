#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <sys/types.h>

//gcc pixel_snake.c -lncurses -pthread

#define SNAKE_LEN 60

typedef struct {
    int x[SNAKE_LEN], y[SNAKE_LEN];
}Snake;

char ch;
int work_flag = 1;
void handler(int none) {
    work_flag = 0;
}

void* thread_funk() {
    int sd;
    int cd;
    struct sockaddr_in addr, remoteaddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    if( sd < 0 ) {
        perror("Error calling socket");
        close(sd);
        return NULL;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2021);
    // addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    addr.sin_addr.s_addr = INADDR_ANY;

    if( bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ) {
        perror("Bind");
        close(sd);
        return NULL;
    }
    
    if( listen(sd, 5) ) {
        perror("Listen");
        close(sd);
        return NULL;
    }

    socklen_t remoteaddr_len;
    if ( 0 > (cd = accept(sd, (struct sockaddr *)&remoteaddr, &remoteaddr_len ))) {
        perror("Accept");
        close(sd);
        close(cd);
        return NULL;
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

void replace(Snake *snake, int length) {
    for(int i = 0; i < length - 1; i++)
    {
        snake->x[i]=snake->x[i+1];
        snake->y[i]=snake->y[i+1];
    }
}

int main(int argc, char *argv[]) {
    int fb;
    struct fb_var_screeninfo info;
    size_t fb_size, map_size, page_size;
    uint32_t *ptr, color = 0x666333cc;

    signal(SIGINT, handler);

    page_size = sysconf(_SC_PAGESIZE);

    if ( 0 > (fb = open("/dev/fb0", O_RDWR))) {
        perror("open");
        return __LINE__;
    }

    if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info)) {
        perror("ioctl");
        close(fb);
        return __LINE__;
    }

    fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
    map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));

    ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if ( MAP_FAILED == ptr ) {
        perror("mmap");
        close(fb);
        return __LINE__;
    }

    initscr();
    noecho();
    keypad(stdscr,TRUE);

    Snake snake;
    for(int i = 0; i < SNAKE_LEN; i++) {
        snake.x[i] = 0; snake.y[i] = 0;
    }

    pthread_t* thread = malloc(sizeof(pthread_t));
    pthread_create(thread, NULL, thread_funk, NULL);
    while(work_flag) {
        for(int i = 0;i < SNAKE_LEN; i++)
            ptr[snake.y[i] * info.xres_virtual + snake.x[i]] = color;

        ptr[snake.y[0] * info.xres_virtual + snake.x[0]] = 0;

        replace(&snake, SNAKE_LEN);

        switch(ch) {
            case 'w':
                snake.y[SNAKE_LEN - 1] = (snake.y[SNAKE_LEN - 1] + info.yres - 1) % info.yres;
                break;
            case 's':
                snake.y[SNAKE_LEN - 1] = (snake.y[SNAKE_LEN - 1] + 1) % info.yres;
                break;
            case 'a':
                snake.x[SNAKE_LEN - 1] = (snake.x[SNAKE_LEN - 1] + info.xres - 1) % info.xres;
                    break;
            case 'd':
                snake.x[SNAKE_LEN - 1]=(snake.x[SNAKE_LEN - 1] + 1) % info.xres;
                break;
            default:
                break;
        }

        usleep(10000);
    }

    pthread_join(*thread, NULL);
    free(thread);
    munmap(ptr, map_size);
    close(fb);
    return 0;
}
