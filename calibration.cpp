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


#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <iostream>

#include "calibration.h"
#include "wiimote.h"



namespace Calibration
{
	SDL_Surface *s;
	
	
	#define MAXTICKS  1500
	#define READY     1
	#define FINISHED  2
	#define FINISHED2 3
	class SandClock
	{
		public:
		
		int total_ticks, last_tick;
		Point point;
		int state;
		
		void init()
		{
			total_ticks = 0;
			last_tick = 0;
			state = READY;
		}
		
		void draw(SDL_Surface *s, int x, int y)
		{
			int dgrs = 360*total_ticks/MAXTICKS;
			if (dgrs > 359)
				dgrs = 359;
			arcRGBA(s, x,y, 80, 0,dgrs, 100,100,100,255); 
		}
		
		void update( Point *p)
		{			
			int ct = SDL_GetTicks();
			int delta = (last_tick == 0) ? 0 : ct - last_tick;
			
			if (p == 0)
			{
				if (delta > 100)
				{
					if (state == FINISHED)
						state = FINISHED2;
					if (delta > 150)
						init();
				}
				return;
			}
			
			point = *p;
			last_tick = ct;
			if (total_ticks < MAXTICKS)
				total_ticks += delta;
			else
				state = FINISHED;
		}
		
		bool finished()
		{
			return (state == FINISHED2);
		}
		
		Point get_point()
		{
			return point;
		}
		
	};
	
	
	

	void draw_point(Point *p)
	{
		aalineRGBA(s, p->x-5, p->y-5, p->x+5, p->y+5, 255,255,255,255);
		aalineRGBA(s, p->x-5, p->y+5, p->x+5, p->y-5, 255,255,255,255);
		
	}

	void draw_square(Point *p)
	{		
		rectangleRGBA(s, p->x-10, p->y-10, p->x+10, p->y+10, 255,255,255,255);

	}

	void draw_cross(int x, int y)
	{
		aalineRGBA(s, x-5, y, x+5, y, 255,255,255,255);
		aalineRGBA(s, x, y-5, x, y+5, 255,255,255,255);

	}

	void calibrate(Wiimote &w)
	{
		SDL_Event e;
		Uint32 black_color, white_color;
		Uint8 *k;
		int state = 0;
		int ok=1;
		int i;
		int t=0;
		float xm1,ym1,xm2,ym2;
		
		SandClock sandClock;
		
		int SIZEX = Scr::getScreenWidth();
		int SIZEY = Scr::getScreenHeight();
		
		SDL_Init(SDL_INIT_VIDEO);
		s = SDL_SetVideoMode(SIZEX,SIZEY,0,SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);
		black_color = SDL_MapRGB(s->format,0,0,0);
		white_color = SDL_MapRGB(s->format,255,0,0);

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
		sandClock.init();
		while(1)
		{
			if (!w.getMsgs())
 				SDL_Delay(50);
			
			if (w.dataReady())
			{
				wiiP = w.getPos();
				sandClock.update(&wiiP);
			}
			else
				sandClock.update(0);
			
			SDL_PollEvent(&e);
			k = SDL_GetKeyState(NULL);
			if (k[SDLK_ESCAPE]) { ok=0; break; }

			if (sandClock.finished()) 
			{
				p_wii[state] = sandClock.get_point();
				state++; 
				sandClock.init(); 
			}

// 			if (state < 4) { p_wii[state] = wiiP; }
			
			if (state >= 4) 
				break;

			rectangleRGBA(s,xm1,ym1,xm2,ym2, 255,255,255,255);

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
			
// 			if (state>0)
// 			{
// 				for (int i=0; i<state; i++)
// 					lineRGBA(s,p_wii[i].x,p_wii[i].y,
// 							   p_wii[i+1].x,p_wii[i+1].y,
// 							   255,255,255,255);
// 			}

			if ((state<4)) 
				draw_square(&p_screen[state]);

			t = ~ t;
			
			sandClock.draw(s, SIZEX/2, SIZEY/2);

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
	}
}


