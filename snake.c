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


typedef struct
{
	int x[40];
	int y[40];
} dlina;
int work_flag = 1, i=0;
char ch;
char ch1;
int pointwin=0;

void* direction()
{
	char g=ch;
	while(ch!='q' || pointwin==1)
	{
		ch1=ch;
		ch=getch();
		switch(ch)
		{
			case'w':
				if(ch1=='s')
				{
					ch=ch1;
				}
				break;
			case's':
				if(ch1=='w')
				{
					ch=ch1;
				}
				break;
			case'd':
				if(ch1=='a')
				{
					ch=ch1;
				}
				break;
			case'a':
				if(ch1=='d')
				{
					ch=ch1;
				}
				break;
			default:
				break;
					
		
		}
		if(ch!='a' && ch!='s' && ch!='w' && ch!='d' && ch!='q')
			ch=g;
	}
		
	return NULL;
}




void handler(int none)
{
	work_flag = 0;
}

int main(int argc, char *argv[])
{
	int fb, x, y, xstep, ystep;
	struct fb_var_screeninfo info;
	size_t fb_size, map_size, page_size;
	uint32_t *ptr, color;
	signal(SIGINT, handler); 

	color = 0x666333cc;
	x = y = 0;
	xstep = ystep = 1;
	page_size = sysconf(_SC_PAGESIZE);
  
	if ( 0 > (fb = open("/dev/fb0", O_RDWR)))
	{
		perror("open");
		return __LINE__;
	}

	if ( (-1) == ioctl(fb, FBIOGET_VSCREENINFO, &info))
	{
		perror("ioctl");
    		close(fb);
    		return __LINE__;
  	}
  
  	fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
  	map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));

  	ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
  	if ( MAP_FAILED == ptr ) 
  	{
		perror("mmap");
		close(fb);
		return __LINE__;
  	}
  	
  	dlina snake;
  	for(i=0;i<40; i++)
  	{
  		//snake.x[i]=i;
  		snake.y[i]=i/8 + 20;
  		snake.x[i]=i%8 + 20;
  	}
  	int taley=22;
  	int talex=19;
   	if( NULL == initscr())
		    return __LINE__;

	noecho();
    	keypad(stdscr,TRUE);

    	pthread_t* threads =(pthread_t*)malloc(sizeof(pthread_t));
    	pthread_create(threads, NULL, direction, NULL);
  	while(ch!='q' && work_flag)
  	{
  		for(i=0;i<40; i++)
    			ptr[snake.y[i] * info.xres_virtual  + snake.x[i]] = color;
    			ptr[taley * info.xres_virtual + talex]=color;
    			usleep(10000);
  		for(i=0;i<40; i++)
    			ptr[snake.y[i] * info.xres_virtual  + snake.x[i]] = 0x00000000;
		switch(ch)
		{
			case 'w':
				taley=snake.y[16];
    				talex=snake.x[16];
				for(i=0;i<40; i++)
  				{
  					snake.y[i]=taley-1 - i%8;
  					snake.x[i]=talex-2 + i/8;
  				}
				break;
			case 's':
				taley=snake.y[16];
    				talex=snake.x[16];
				for(i=0;i<40; i++)
  				{
  					snake.y[i]=taley+1 + i%8;
  					snake.x[i]=talex+2 - i/8;
  				}
				break;
			case 'a':
				taley=snake.y[16];
    				talex=snake.x[16];
				for(i=0;i<40; i++)
  				{
  					snake.y[i]=taley+2 - i/8;
  					snake.x[i]=talex-1 - i%8;
  				}
					break;
			case 'd':
				taley=snake.y[16];
    				talex=snake.x[16];
				for(i=0;i<40; i++)
  				{
  					snake.y[i]=taley-2 + i/8;
  					snake.x[i]=talex+1 + i%8;
  				}
				break;
			default:
				break;
  				
		}
		for(i=0;i<40; i++)
  		{
  			if(ptr[snake.y[i] * info.xres_virtual  + snake.x[i]] == color)
			pointwin=1;
 					
		}
  		if(pointwin==1)
  		{
  			perror("win");
  			break;
  		}
  			

  	}
 	munmap(ptr, map_size);
  	close(fb);
  	return 0;
}