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
char ch1 = 0;
char ch2 = 0;
int work_flag = 1, i=0;
int pointwin1=0, pointwin2=0;
char k = 0,g = 0;

void* direction()
{
	while((ch1!='q' && ch2!='q') || pointwin1==1 || pointwin2==1)
	{
		ch1=getch();
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
	uint32_t *ptr, color1, color2;
	signal(SIGINT, handler); 

	color1 = 0xd63a1e ;
	color2 = 0x00FFFF;
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
  	for(i=0;i<40; i++)
  	{
  		snake2.y[i]=info.yres-20 - i/8;
  		snake2.x[i]=info.xres-20 - i%8;
  	}
  	int taley1=snake1.y[16];
	int talex1=snake1.x[16];
	int taley2=snake2.y[16];
	int talex2=snake2.x[16];
   	if( NULL == initscr())
		    return __LINE__;

	noecho();
    	keypad(stdscr,TRUE);

    	pthread_t* threads =(pthread_t*)malloc(2*sizeof(pthread_t));
    	pthread_create(threads, NULL, direction, NULL);
  	while((ch1!='q'|| ch2!='q') && work_flag)
  	{
  		for(i=0;i<40; i++)
  			{
    			ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] = color1;
    			ptr[snake2.y[i] * info.xres_virtual  + snake2.x[i]] = color2;
    			}
    			ptr[taley1 * info.xres_virtual + talex1]=color1;
    			ptr[taley2 * info.yres_virtual + talex2]=color2;
    			usleep(10000);
  		for(i=0;i<40; i++)
  			{
    			ptr[snake2.y[i] * info.xres_virtual  + snake2.x[i]] = 0x00000000;
    			ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] = 0x00000000;
			}
		if(ch1!='a' && ch1!='s' && ch1!='w' && ch1!='d' && ch1!='q')
			ch1=g;
		switch(ch1)
		{
			case 'w':
				if(g=='s')
				{
					ch1=g;
					break;
				}
				taley1--;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1-1 - i%8;
  					snake1.x[i]=talex1-2 + i/8;
  				}
				break;
			case 's':
				if(g=='w')
				{
					ch1=g;
					break;
				}
				taley1++;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1+1 + i%8;
  					snake1.x[i]=talex1+2 - i/8;
  				}
				break;
			case 'a':

    				if(g=='d')
				{
					ch1=g;
					break;
				}
				talex1--;
				for(i=0;i<40; i++)
  				{
  					snake1.y[i]=taley1+2 - i/8;
  					snake1.x[i]=talex1-1 - i%8;
  				}
					break;
			case 'd':
    				if(g=='a')
				{
					ch1=g;
					break;
				}
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
		g=ch1;
		if(ch2!='a' && ch2!='s' && ch2!='w' && ch2!='d' && ch2!='q')
			ch2=k;
		switch(ch2)
		{
			case 'w':
				if(k=='s')
				{
					ch2=k;
					break;
				}
				taley2--;
				for(i=0;i<40; i++)
  				{
  					snake2.y[i]=taley2-1 - i%8;
  					snake2.x[i]=talex2-2 + i/8;
  				}
				break;
			case 's':
				if(k=='w')
				{
					ch2=k;
					break;
				}
				taley2++;
				for(i=0;i<40; i++)
  				{
  					snake2.y[i]=taley2+1 + i%8;
  					snake2.x[i]=talex2+2 - i/8;
  				}
				break;
			case 'a':
    				if(k=='d')
				{
					ch2=k;
					break;
				}
    				talex2--;
				for(i=0;i<40; i++)
  				{
  					snake2.y[i]=taley2+2 - i/8;
  					snake2.x[i]=talex2-1 - i%8;
  				}
					break;
			case 'd':
    				if(k=='a')
				{
					ch2=k;
					break;
				}
    				talex2++;
				for(i=0;i<40; i++)
  				{
  					snake2.y[i]=taley2-2 + i/8;
  					snake2.x[i]=talex2+1 + i%8;
  				}
				break;
			default:
				break;
  				
		}
		k=ch2;
		for(i=0;i<40; i++)
  		{
  			if(ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] == color1 || ptr[snake1.y[i] * info.xres_virtual  + snake1.x[i]] == color2 || snake1.y[i]>info.yres || snake1.y[i]<1 || snake1.x[i]>info.xres || snake1.x[i]<1 || (snake1.y[i]==snake2.y[i] && snake1.x[i]==snake2.x[i]))
				pointwin1=1;
			if(ch1==0)
				pointwin1=0;
 					
		}
		for(i=0;i<40; i++)
  		{
  			if(ptr[snake2.y[i] * info.xres_virtual  + snake2.x[i]] == color1 || ptr[snake2.y[i] * info.xres_virtual  + snake2.x[i]] == color1 || snake2.y[i]>info.yres || snake2.y[i]<1 || snake2.x[i]>info.xres || snake2.x[i]<1 || (snake1.y[i]==snake2.y[i] && snake1.x[i]==snake2.x[i]))
				pointwin2=1;
			if(ch2==0)
				pointwin2=0;
 					
		}
  		if(pointwin1==1 && pointwin2==1)
  		{
  			printf("Draw");
  			break;
  		}
  		
  		if(pointwin1==1 && pointwin2==0)
  		{	
  			printf( "Win Player 2");
  			break;
  		}
  		
  		if(pointwin1==0 && pointwin2==1)
  		{
  			printf( "Win Player 1");
  			break;
  		}
  			

  	}
 	munmap(ptr, map_size);
 	endwin();
  	close(fb);
  	return 0;
}