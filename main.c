#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "controller.h"

void *thread_server(void *thread_data)
{
    Server_data *data = (Server_data *)thread_data;
    controller_server(data->local_port, data->remote_port, data->remote_ip, data->ch1, data->ch2, data->work_flag);
    return NULL;
}

void *thread_client(void *thread_data)
{
    Client_data *data = (Client_data *)thread_data;
    controller_client(data->remote_ip, data->local_port, data->remote_port, data->work_flag);
    return NULL;
}

typedef struct
{
    int x[40];
    int y[40];
} dlina;

int main(int argc, char *argv[])
{
    char remote_ip[15];
    sscanf(argv[2], "%s", remote_ip);

    pthread_t *thread_1 = malloc(sizeof(pthread_t));
    pthread_t *thread_2 = malloc(sizeof(pthread_t));
    uint8_t work_flag = 1;
    int pointwin1 = 0, pointwin2 = 0, i = 0;
    char k = 0, g = 0;
    char ch1 = 0, ch2 = 0;

    Server_data server_data = {
        .local_port = 2021,
        .remote_port = 12345,
        .remote_ip = remote_ip,
        .ch1 = &ch1,
        .ch2 = &ch2,
        .work_flag = &work_flag};

    Client_data client_data = {
        .remote_ip = remote_ip,
        .local_port = 2021,
        .remote_port = 12345,
        .work_flag = &work_flag};

    initscr();
    noecho();

    pthread_create(thread_1, NULL, thread_server, &server_data);
    sleep(1);
    pthread_create(thread_2, NULL, thread_client, &client_data);

    int fb, x, y, xstep, ystep;
    struct fb_var_screeninfo info;
    size_t fb_size, map_size, page_size;
    uint32_t *ptr, color1, color2, color3;
    int perx, pery;
    sscanf(argv[1], "%dx%d", &perx, &pery);
    // signal(SIGINT, handler);
    color1 = 0xc64444;
    color2 = 0x5144c6;
    color3 = 0xc144c6;
    x = y = 0;
    xstep = ystep = 1;
    page_size = sysconf(_SC_PAGESIZE);

    if (0 > (fb = open("/dev/fb0", O_RDWR)))
    {
        perror("open");
        return __LINE__;
    }

    if ((-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info))
    {
        perror("ioctl");
        close(fb);
        return __LINE__;
    }

    fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
    map_size = (fb_size + (page_size - 1)) & (~(page_size - 1));

    ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (MAP_FAILED == ptr)
    {
        perror("mmap");
        close(fb);
        return __LINE__;
    }

    dlina snake1, snake2;
    for (i = 0; i < 40; i++)
    {
        snake1.y[i] = info.yres / 2 - pery / 2 + i / 8 + 20;
        snake1.x[i] = info.xres / 2 - perx / 2 + i % 8 + 20;
    }
    for (i = 0; i < 40; i++)
    {
        snake2.y[i] = info.yres / 2 + pery / 2 - 20 - i / 8;
        snake2.x[i] = info.xres / 2 + perx / 2 - 20 - i % 8;
    }
    int taley1 = snake1.y[16];
    int talex1 = snake1.x[16];
    int taley2 = snake2.y[16];
    int talex2 = snake2.x[16];

    // if( NULL == initscr())
    //	    return __LINE__;

    // noecho();
    // keypad(stdscr,TRUE);
    // pthread_t* threads =(pthread_t*)malloc(2*sizeof(pthread_t));
    // pthread_create(threads, NULL, direction, NULL);
    while ((ch1 != 'q' || ch2 != 'q') && (work_flag==1))
    {
	for(int i=0; i<perx;i++)
  		{
  			ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres-perx)/2+i]=color3;
  			ptr[(info.yres+pery)/2 * info.xres_virtual + (info.xres-perx)/2+i]=color3;
  		}
  	for(int i=0;i<pery;i++)
  		{
  			ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres-perx)/2+i*info.xres_virtual]=color3;
  			ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres+perx)/2+i*info.xres_virtual]=color3;
  		}
  	for(int i=0;i<40; i++)
  		{
    			ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] = color1;
    			ptr[snake2.y[i] * info.xres_virtual  + snake2.x[i]] = color2;
    		}
    	ptr[taley1 * info.xres_virtual + talex1]=color1;
    	ptr[taley2 * info.xres_virtual + talex2]=color2;
        if (ch1 == 0 && ch2 == 0) continue;
        usleep(30000);
        for (i = 0; i < 40; i++)
        {
            ptr[snake2.y[i] * info.xres_virtual + snake2.x[i]] = 0x00000000;
            ptr[snake1.y[i] * info.xres_virtual + snake1.x[i]] = 0x00000000;
        }
        if (ch1 != 'a' && ch1 != 's' && ch1 != 'w' && ch1 != 'd' && ch1 != 'q')
            ch1 = g;
        switch (ch1)
        {
        case 'w':
            if (g == 's')
            {
                ch1 = g;
                break;
            }
            taley1--;
            for (i = 0; i < 40; i++)
            {
                snake1.y[i] = taley1 - 1 - i % 8;
                snake1.x[i] = talex1 - 2 + i / 8;
            }
            break;
        case 's':
            if (g == 'w')
            {
                ch1 = g;
                break;
            }
            taley1++;
            for (i = 0; i < 40; i++)
            {
                snake1.y[i] = taley1 + 1 + i % 8;
                snake1.x[i] = talex1 + 2 - i / 8;
            }
            break;
	case 'a':

            if (g == 'd')
            {
                ch1 = g;
                break;
            }
            talex1--;
            for (i = 0; i < 40; i++)
            {
                snake1.y[i] = taley1 + 2 - i / 8;
                snake1.x[i] = talex1 - 1 - i % 8;
            }
            break;
        case 'd':
            if (g == 'a')
            {
                ch1 = g;
                break;
            }
            talex1++;
            for (i = 0; i < 40; i++)
            {
                snake1.y[i] = taley1 - 2 + i / 8;
                snake1.x[i] = talex1 + 1 + i % 8;
            }
            break;
        default:
            break;
        }
        g = ch1;
        if (ch2 != 'a' && ch2 != 's' && ch2 != 'w' && ch2 != 'd' && ch2 != 'q')
            ch2 = k;
        switch (ch2)
        {
        case 'w':
            if (k == 's')
            {
                ch2 = k;
                break;
            }
            taley2--;
            for (i = 0; i < 40; i++)
            {
                snake2.y[i] = taley2 - 1 - i % 8;
                snake2.x[i] = talex2 - 2 + i / 8;
            }
            break;
        case 's':
            if (k == 'w')
            {
                ch2 = k;
                break;
            }
            taley2++;
            for (i = 0; i < 40; i++)
            {
                snake2.y[i] = taley2 + 1 + i % 8;
                snake2.x[i] = talex2 + 2 - i / 8;
            }
            break;
        case 'a':
            if (k == 'd')
            {
                ch2 = k;
                break;
            }
            talex2--;
            for (i = 0; i < 40; i++)
            {
                snake2.y[i] = taley2 + 2 - i / 8;
                snake2.x[i] = talex2 - 1 - i % 8;
            }
            break;
        case 'd':
            if (k == 'a')
            {
                ch2 = k;
                break;
            }
            talex2++;
            for (i = 0; i < 40; i++)
            {
                snake2.y[i] = taley2 - 2 + i / 8;
                snake2.x[i] = talex2 + 1 + i % 8;
            }
            break;
        default:
            break;
        }
        k = ch2;
        for (i = 0; i < 40; i++)
        {
            if (ptr[snake1.y[i] * info.xres_virtual + snake1.x[i]] == color1 || ptr[snake1.y[i] * info.xres_virtual + snake1.x[i]] == color2 || ptr[snake1.y[i] * info.xres_virtual + snake1.x[i]] == color3 || (snake1.y[i] == snake2.y[i] && snake1.x[i] == snake2.x[i]))
                pointwin1 = 1;
            if (ch1 == 0)
                pointwin1 = 0;
        }
        for (i = 0; i < 40; i++)
        {
            if (ptr[snake2.y[i] * info.xres_virtual + snake2.x[i]] == color1 || ptr[snake2.y[i] * info.xres_virtual + snake2.x[i]] == color2 || ptr[snake2.y[i] * info.xres_virtual + snake2.x[i]] == color3 || (snake1.y[i] == snake2.y[i] && snake1.x[i] == snake2.x[i]))
                pointwin2 = 1;
            if (ch2 == 0)
                pointwin2 = 0;
        }
        if ((pointwin1 == 1 && pointwin2 == 1)|| (work_flag==4))
        {
            work_flag = 4;
            for(i=0;i<perx/2+1;i++)
  		{
  			for(int j=0; j<pery+1;j++)
  				{
					 ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres-perx)/2+i+j*info.xres_virtual]=color1;
					 ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres)/2+i+j*info.xres_virtual]=color2;			
  				}
  		}
            break;
        }

        if ((pointwin1 == 1 && pointwin2 == 0)||(work_flag==3))
        {
            work_flag = 3;
            for(i=0;i<perx+1;i++)
  			{
 			for(int j=0; j<pery+1;j++)
  				{
					 ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres-perx)/2+i+j*info.xres_virtual]=color2;			
  				}
  			}
            break;
        }

        if ((pointwin1 == 0 && pointwin2 == 1) && (work_flag==2))
        {
            work_flag = 2;
	for(i=0;i<perx+1;i++)
  			{
  				for(int j=0; j<pery+1;j++)
  				{
					 ptr[(info.yres-pery)/2 * info.xres_virtual + (info.xres-perx)/2+i+j*info.xres_virtual]=color1;			
  				}
  			}  
  	break;
        }
    }

    munmap(ptr, map_size);
    // endwin();
    close(fb);

    pthread_join(*thread_1, NULL);
    pthread_join(*thread_2, NULL);

    free(thread_1);
    free(thread_2);

    endwin();
}
