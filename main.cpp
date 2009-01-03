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

#include <iostream>


#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>


#include <cwiid.h>



#include "point.h"
#include "calibration.h"
#include "wiimote.h"
#include "timer.h"


class Click
{
	public:
		
	typedef enum { LEFT = 1, RIGHT=3, DOUBLE } but_t;
	
	protected:
	
	but_t but;
	int initialTime;
	
	public:
	
	Click(but_t b)
	{
		initialTime = Timer::getTicks();
		but = b;
		button(true);
	}
	
	bool refresh(bool evt)
	{
		int t = Timer::getTicks();
		
		if (but == DOUBLE)
			return false;
		
		if (evt)
		{
			initialTime = t;
			return true;
		}
		else
		{
			if ((t - initialTime) > 50)
			{
				button(false);
				return false;
			}
		}
		return true;
	}
	
	void button(bool press)
	{
		Display* display = XOpenDisplay(0);
		
		switch(but)
		{
			case LEFT:
				XTestFakeButtonEvent(display, 1, (int) press, 0);
				break;
				
			case RIGHT:
				XTestFakeButtonEvent(display, 3, (int) press, 0);
				break;
				
			case DOUBLE:
				XTestFakeButtonEvent(display, 1, 1, 0);
				XTestFakeButtonEvent(display, 1, 0, 40);
				XTestFakeButtonEvent(display, 1, 1, 80);
				XTestFakeButtonEvent(display, 1, 0, 120);
				break;
		}
		
		XCloseDisplay(display);
	}
	
	
	
};


class FakeCursor
{
	protected:
	
	Display *display;
	Wiimote *wii;
	
	typedef enum { ACTIVE, INACTIVE } state_t;
	state_t state;
	
	Click::but_t clickType;
	Click *click;

	public:
	
	FakeCursor()
	{
		state = INACTIVE;
		wii = 0;
		click = 0;
		setClickType(Click::LEFT);
	}
	
	void attachWiimote(Wiimote *wiim)
	{
		wii = wiim;
	}
	
	Wiimote *getWii()
	{
		return wii;
	}
	
	void activate()
	{
		state = ACTIVE;
	}
	
	void deactivate()
	{
		state = INACTIVE;
	}
	
	void setClickType(Click::but_t c)
	{
		clickType = c;
	}
	
	bool checkLimits(Point p)
	{
		if (p.x < 0)
		{
			setClickType(Click::RIGHT);
			return false;	
		}
		if (p.y < 0)
		{
			setClickType(Click::DOUBLE);
			return false;
		}
		return true;
	}
	
	void update()
	{		
		if (!wii)
			return;
		
		if (state != ACTIVE)
			return;
		
		if (wii->dataReady())
		{
			Point p = wii->getPos();
			if ((!click) && (checkLimits(p) == false))
				return;
			
			move(p);
			
			if (!click)
				click = new Click(clickType);
			else
				click->refresh(true);
		}
		else
		{
			if (click)
			{
				if (!click->refresh(false))
				{
					clickType = Click::LEFT;
					delete click;
					click = 0;
				}
			}
		}
		
	}
	
	
	void move(Point p)
	{
		display = XOpenDisplay(0);
		XTestFakeMotionEvent(display,-1,p.x,p.y,0);
		XCloseDisplay(display);
	}
	
};







int main(int argc,char *argv[])
{
	Wiimote wiim;
	
	Timer::start();
	
	if (!wiim.connection())
		exit(1);
	
	if (!Calibration::do_calibration(&wiim))
		exit(1);
	
	wiim.getMsgs();
	
	FakeCursor cursor;
	cursor.attachWiimote(&wiim);
	cursor.activate();
	
	while (!wiim.isButtonPressed())
	{
		wiim.getMsgs();
		cursor.update();
	}
	
	wiim.endConnection();

	return 0;
}



