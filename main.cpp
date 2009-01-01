/* Copyright (C) 2008 Pere Negre
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL.h>
#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <sys/time.h>

extern "C" {
	#include "matrix.h"
}





class Point
{
	public:
	int x, y;
};


class Wiimote
{
	protected:

	typedef enum { CALIBRATED, NON_CALIBRATED } state_t;
	state_t state;
	Point p;
	// Calibration data
	float h11,h12,h13,h21,h22,h23,h31,h32;

	public:

	Wiimote()
	{
		state = NON_CALIBRATED;
	}

	void irData(int *v)
	{
		switch(state)
		{
			case (NON_CALIBRATED):
				p.x = v[0];
				p.y = v[1];
				break;
			
			case (CALIBRATED):
				p.x =  (int) ( ( (float)  (h11*v[0] + h12*v[1] + h13) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
				p.y =  (int) ( ( (float)  (h21*v[0] + h22*v[1] + h23) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
				break;
		}
	}
	
	void calibrate(float H11, float H12, float H13, float H21, float H22, float H23, float H31, float H32)
	{
		h11 = H11; h12 = H12; h13 = H13;
		h21 = H21; h22 = H22; h23 = H23;
		h31 = H31; h32 = H32;
		state = CALIBRATED;
	}

	Point getPos()
	{
		return p;
	}
};


Wiimote wiim;





extern int wii_connect(char *mac);
extern void wii_disconnect();

// int rx=0, ry=0;

int SIZEX;
int SIZEY;

SDL_Surface *s;

int ready=0, can_exit = 0;

int event_has_occurred = 0;

char mac[100];

Display *display;

#define MAX_WII_X 1020
#define MAX_WII_Y 760


void buttonpress()
{
	can_exit = 1;
}


void infrared_data(int *v)
{
	wiim.irData(v);
}



void read_parameters(int argc, char *argv[])
{
//===========================================SIZE
        display = XOpenDisplay(0);
	int screen = DefaultScreen(display);	
	SIZEX = DisplayWidth(display,screen);
	SIZEY = DisplayHeight(display,screen);
	XCloseDisplay(display);

	printf("sizex = %d\n",SIZEX);
	printf("sizey = %d\n",SIZEY);

//============================================MAC
	if(argc>1){
		strcpy(mac,argv[1]);
		//str2ba(argv[1],&mac);
		printf("mac = %s\n",mac);
	}
	else{
		//mac = *BDADDR_ANY;
		mac[0]='#';
		printf("mac = ANY \n");
	}

}





void pixel(int x, int y)
{
	Uint32 *m;
	y *= s->w;
	m = (Uint32*) s->pixels + y + x;
	*m = SDL_MapRGB(s->format,255,255,255);
}




void draw_point(Point *p)
{
	int i;
	for (i=p->x-10; i<p->x+10; i++)
		pixel(i,p->y);
	
	for (i=p->y-10; i<p->y+10; i++)
		pixel(p->x,i);
}


void draw_square(Point *p)
{
	int i;
	for (i=p->x-10; i<p->x+10; i++)
		pixel(i,p->y+10), pixel(i,p->y-10);

        for (i=p->y-10; i<p->y+10; i++)
                pixel(p->x-10,i), pixel(p->x+10,i);

}


void draw_cross(int x, int y)
{
	int x1,y1;
	int i;
	for(i=-5; i<=5; i++)
		pixel(x,y+i);
	for(i=-5; i<=5; i++)
		pixel(x+i,y);
}



// void printpoints()
// {
// 	int i;
// 	for (i=0; i<4; i++)
// 		printf("Point %d --> (%d,%d) === (%d,%d)\n", 
// 			i,
// 			p_screen[i].x,
// 			p_screen[i].y,
// 			p_wii[i].x,
// 			p_wii[i].y);
// }



void do_calcs(Point p_screen[], Point p_wii[])
{
	int i;

	matrix_t *m, *n, *r;

	m = matrixNew(8,8);
	n = matrixNew(1,8);

	for (i=0; i<4; i++)
	{
		matrixSetElement(n, (float) p_screen[i].x, 0, i*2);
		matrixSetElement(n, (float) p_screen[i].y, 0, i*2 + 1);
	}

	for (i=0; i<4; i++)
	{
		matrixSetElement(m, (float) p_wii[i].x, 0, i*2);
		matrixSetElement(m, (float) p_wii[i].y, 1, i*2);
		matrixSetElement(m, (float) 1, 2, i*2);
		matrixSetElement(m, (float) 0, 3, i*2);
		matrixSetElement(m, (float) 0, 4, i*2);
		matrixSetElement(m, (float) 0, 5, i*2);
		matrixSetElement(m, (float) (-p_screen[i].x * p_wii[i].x), 6, i*2);
		matrixSetElement(m, (float) (-p_screen[i].x * p_wii[i].y), 7, i*2);

		matrixSetElement(m, (float) 0, 0, i*2+1);
		matrixSetElement(m, (float) 0, 1, i*2+1);
		matrixSetElement(m, (float) 0, 2, i*2+1);
		matrixSetElement(m, (float) p_wii[i].x, 3, i*2+1);
		matrixSetElement(m, (float) p_wii[i].y, 4, i*2+1);
		matrixSetElement(m, (float) 1, 5, i*2+1);
		matrixSetElement(m, (float) (-p_screen[i].y * p_wii[i].x), 6, i*2+1);
		matrixSetElement(m, (float) (-p_screen[i].y * p_wii[i].y), 7, i*2+1);
	}

	matrixInverse(m);
	r = matrixMultiply(m,n);
	
	float h11,h12,h13,h21,h22,h23,h31,h32;

	h11 = matrixGetElement(r,0,0);
	h12 = matrixGetElement(r,0,1);
	h13 = matrixGetElement(r,0,2);
	h21 = matrixGetElement(r,0,3);
	h22 = matrixGetElement(r,0,4);
	h23 = matrixGetElement(r,0,5);
	h31 = matrixGetElement(r,0,6);
	h32 = matrixGetElement(r,0,7);

	wiim.calibrate(h11,h12,h13,h21,h22,h23,h31,h32);
	
	matrixFree(m);
	matrixFree(n);
	matrixFree(r);

}








void movePointer(int x, int y)
{
	display = XOpenDisplay(0);
	XTestFakeMotionEvent(display,-1,x,y,0);
	XCloseDisplay(display);
}

void button(int p)
{
	display = XOpenDisplay(0);
	XTestFakeButtonEvent(display,1,p,0);
	//printf("BUTTON!! %d\n",p);
	XCloseDisplay(display);
}


void the_end()
{
	wii_disconnect();
	exit(0);
}

static struct timeval tst,tend;
static struct timezone tz;

unsigned long myGetTicks()
{
	static double t1,t2;

	gettimeofday(&tend,&tz);
	t1 = (double) tst.tv_sec*1000 + (double) tst.tv_usec/1000;
	t2 = (double) tend.tv_sec*1000 + (double) tend.tv_usec/1000;
	return (unsigned long int) (t2-t1);
}

static void myStartTimer()
{
	gettimeofday(&tst,&tz);
}



void update_cursor()
{
	static int delta,t;
	static int lastevent=0;

	t = myGetTicks();
	if (event_has_occurred)
	{
		Point p = wiim.getPos();
		event_has_occurred=0;
		movePointer(p.x, p.y); 
		if (lastevent == 0) { button(1); }
		lastevent = 1;
		delta = t; 
	}
	else
	{
		if ( (lastevent==1) && ((myGetTicks() - delta)>50)) 
			{ button(0); lastevent = 0; }
	}
}


int main(int argc,char *argv[])
{
	SDL_Event e;
	Uint32 black_color;
	Uint8 *k;
	int state = 0;
	int ok=1;
	int i;
	int t=0;
	float xm1,ym1,xm2,ym2;

	if(argc>2)
	{
		printf("ERROR: \n       Usage demo <mac> \n");
		return 0;
	}
	
	myStartTimer();

	read_parameters(argc,argv);
	
	if (wii_connect(mac) == 0)
		exit(1);
	
	SDL_Init(SDL_INIT_VIDEO);
	s = SDL_SetVideoMode(SIZEX,SIZEY,0,SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);
	black_color = SDL_MapRGB(s->format,0,0,0);

	Point p_screen[4];
	
	p_screen[0].x = 50;	
	p_screen[0].y = 50;

	p_screen[1].x = SIZEX - 50;
	p_screen[1].y = 50;

	p_screen[2].x = 50;
	p_screen[2].y = SIZEY - 50;

	p_screen[3].x = SIZEX - 50;
	p_screen[3].y = SIZEY - 50;
	
	Point p_wii[4];

	SDL_FillRect(s,0,black_color);

	xm1 = SIZEX / 2 - 100;
	xm2 = xm1 + 200;
	ym1 = SIZEY / 2 - 100;
	ym2 = ym1 + 200;

	Point wiiP;
	t = 0;
	while(1)
	{
		wiiP = wiim.getPos();
		
		SDL_PollEvent(&e);
		k = SDL_GetKeyState(NULL);
		if (k[SDLK_ESCAPE]) { ok=0; break; }

		if (k[SDLK_SPACE]) { state++; k[SDLK_SPACE]=0; }

		if (state < 4) { p_wii[state] = wiiP; }
		
		if (state >= 4) 
			break;

		for (i = (int) xm1; i < (int) xm2; i++)
			pixel(i,ym1), pixel(i,ym2);

		for (i = (int) ym1; i < (int) ym2; i++)
			pixel(xm1,i), pixel(xm2,i);

		draw_cross(
			xm1 + (int) ( ((float) wiiP.x / (float) MAX_WII_X )*200),
			ym2 - (int) ( ((float) wiiP.y / (float) MAX_WII_Y )*200)
		);

		draw_point(&p_screen[0]);	
		draw_point(&p_screen[1]);	
		draw_point(&p_screen[2]);	
		draw_point(&p_screen[3]);	

		if (state<4)
			for(i=0; i<state; i++)
				draw_square(&p_screen[i]);

		if ((state<4) && (t)) 
			draw_square(&p_screen[state]);

		t = ~ t; 

		//SDL_UpdateRect(s,0,0,0,0);
		SDL_Flip(s);
		SDL_Delay(100);
		SDL_FillRect(s,0,black_color);
	}
	
	printf("Quitting SDL..");
	SDL_FreeSurface(s);
	SDL_Quit();	
	printf("Done\n");

	if (!ok)
		the_end();

// 	printpoints();
	
	printf("Calculating coefficients...");
	do_calcs(p_screen, p_wii);
	printf("Done!\n");
	
	ready = 1;
	while (!can_exit)
		sleep(1);

	the_end();
	return 0;
}



