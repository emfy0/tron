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
char ch = 0;
int work_flag = 1, i=0;
char ch1;
int pointwin1=0;

void* direction()
{
	char g=ch;
	while(ch!='q' || pointwin1==1)
	{
		char k=ch;
		ch=getch();
		switch(ch)
		{
			case'w':
				if(k=='s')
				{
					ch=k;
				}
				break;
			case's':
				if(k=='w')
				{
					ch=k;
				}
				break;
			case'd':
				if(k=='a')
				{
					ch=k;
				}
				break;
			case'a':
				if(k=='d')
				{
					ch=k;
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
  	
  	dlina snake1, snake2;
  	for(i=0;i<40; i++)
  	{
  		snake1.y[i]=i/8 + 20;
  		snake1.x[i]=i%8 + 20;
  	}
  	int taley1=snake1.y[16];
  	int talex1=snake1.x[16];
   	if( NULL == initscr())
		    return __LINE__;

	noecho();
    	keypad(stdscr,TRUE);

    	pthread_t* threads =(pthread_t*)malloc(sizeof(pthread_t));
    	pthread_create(threads, NULL, direction, &ch);
  	while(ch!='q' && work_flag)
  	{
  		for(i=0;i<40; i++)
    			ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] = color;
    			ptr[taley1 * info.xres_virtual + talex1]=color;
    			usleep(62500);
  		for(i=0;i<40; i++)
    			ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] = 0x00000000;
		switch(ch)
		{
			case 'w':
				taley1--;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1-1 - i%8;
  					snake1.x[i]=talex1-2 + i/8;
  				}
				break;
			case 's':
				taley1++;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1+1 + i%8;
  					snake1.x[i]=talex1+2 - i/8;
  				}
				break;
			case 'a':
    				talex1--;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1+2 - i/8;
  					snake1.x[i]=talex1-1 - i%8;
  				}
					break;
			case 'd':
    				talex1++;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1-2 + i/8;
  					snake1.x[i]=talex1+1 + i%8;
  				}
				break;
			default:
				break;
  				
		}
		for(i=0;i<40; i++)
  		{
  			if(ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] == color || snake1.y[i]>info.yres || snake1.y[i]<1 || snake1.x[i]>info.xres || snake1.x[i]<1)
				pointwin1=1;
			if(ch==0)
				pointwin1=0;
 					
		}
  		if(pointwin1==1)
  		{
  			perror("win");
  			break;
  		}
  			

  	}
 	munmap(ptr, map_size);
  	close(fb);
  	return 0;
}