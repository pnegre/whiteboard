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


#include <sys/time.h>
#include <X11/Xlib.h>

#include "common.h"

namespace Scr
{
	int x=0;
	int y=0;
	
	int getScreenWidth()
	{
		if (x)
			return x;
		
		Display* display = XOpenDisplay(0);
		int screen = DefaultScreen(display);
		x = DisplayWidth(display,screen);
		XCloseDisplay(display);
		return x;
	}
	
	int getScreenHeight()
	{
		if (y)
			return y;
		
		Display* display = XOpenDisplay(0);
		int screen = DefaultScreen(display);
		y = DisplayHeight(display,screen);
		XCloseDisplay(display);
		return y;
	}
}


namespace Timer
{
	struct timeval tst,tend;
	struct timezone tz;

	unsigned long getTicks()
	{
		static double t1,t2;

		gettimeofday(&tend,&tz);
		t1 = (double) tst.tv_sec*1000 + (double) tst.tv_usec/1000;
		t2 = (double) tend.tv_sec*1000 + (double) tend.tv_usec/1000;
		return (unsigned long int) (t2-t1);
	}

	void start()
	{
		gettimeofday(&tst,&tz);
	}
}




#include <SDL.h>
#include <iostream>

#include "calibration.h"
#include "wiimote.h"



namespace Calibration
{
	SDL_Surface *s;
	
	void pixel(int x, int y)
	{
		Uint32 *m;
		y *= s->w;
		m = (Uint32*) s->pixels + y + x;
		SDL_LockSurface(s);
		*m = SDL_MapRGB(s->format,255,255,255);
		SDL_UnlockSurface(s);
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

	bool do_calibration(Wiimote &w)
	{
		SDL_Event e;
		Uint32 black_color;
		Uint8 *k;
		int state = 0;
		int ok=1;
		int i;
		int t=0;
		float xm1,ym1,xm2,ym2;
		
		int SIZEX = Scr::getScreenWidth();
		int SIZEY = Scr::getScreenHeight();
		
		SDL_Init(SDL_INIT_VIDEO);
		s = SDL_SetVideoMode(SIZEX,SIZEY,0,SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);
		black_color = SDL_MapRGB(s->format,0,0,0);

		Point p_screen[4];
		Point p_wii[4];
		
		p_screen[0].x = 50;	
		p_screen[0].y = 50;

		p_screen[1].x = SIZEX - 50;
		p_screen[1].y = 50;
		
		p_screen[2].x = SIZEX - 50;
		p_screen[2].y = SIZEY - 50;

		p_screen[3].x = 50;
		p_screen[3].y = SIZEY - 50;

		SDL_FillRect(s,0,black_color);

		xm1 = SIZEX / 2 - 100;
		xm2 = xm1 + 200;
		ym1 = SIZEY / 2 - 100;
		ym2 = ym1 + 200;

		Point wiiP;
		wiiP.x = wiiP.y = 0;
		t = 0;
		while(1)
		{
			if (w.getMsgs())
			{
// 				std::cout << "IR MSG\n";
				wiiP = w.getPos();
			}
 			else
 				SDL_Delay(50);
			
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

			if ((state<4)) 
				draw_square(&p_screen[state]);

			t = ~ t; 

			//SDL_UpdateRect(s,0,0,0,0);
			SDL_Flip(s);
			//SDL_Delay(100);
			SDL_FillRect(s,0,black_color);
		}
		
		std::cout << "Quitting SDL..";
		SDL_FreeSurface(s);
		SDL_Quit();	
		std::cout << "Done\n";

		if (!ok)
			throw Error();
		
		std::cout << "Calculating coefficients...";
		w.calibrate(p_screen, p_wii);
		std::cout << "Done!\n";
		
		return true;
	}
}








