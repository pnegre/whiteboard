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

#include "cursor.h"
#include "timer.h"

#include <iostream>
#include <list>


namespace Scr
{
	int getScreenWidth();
	int getScreenHeight();
}


#define MAXPOINTS 5

namespace Filter
{
	std::list<Point> cache;
	
	void init()
	{
		cache.clear();
	}
	
	Point process(const Point p)
	{
		cache.push_back(p);
		if (cache.size() > MAXPOINTS)
			cache.pop_front();
		
		Point ret;
		ret.x = ret.y = 0;
		
		for (std::list<Point>::const_iterator i = cache.begin(); 
			i != cache.end(); ++i) 
		{
			ret.x += i->x;
			ret.y += i->y;
		}
		
		// We need to cast cache.size() because if not, the result
		// is always unsigned
		ret.x /= (int) cache.size();
		ret.y /= (int) cache.size();
		
		return ret;
	}
}




Click::Click(but_t b)
{
	initialTime = Timer::getTicks();
	but = b;
	button(true);
	Filter::init();
}

bool Click::refresh(bool evt)
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

void Click::button(bool press)
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
		
		case NOCLICK:
			break;
	}
	
	XCloseDisplay(display);
}



// FAKECURSOR ///////////////////////////////////////////////////////////////////



FakeCursor::FakeCursor()
{
	state = INACTIVE;
	wii = 0;
	click = 0;
	setClickType(Click::LEFT);
}

void FakeCursor::attachWiimote(Wiimote *wiim)
{
	wii = wiim;
}

Wiimote *FakeCursor::getWii()
{
	return wii;
}

void FakeCursor::activate()
{
	state = ACTIVE;
}

void FakeCursor::deactivate()
{
	state = INACTIVE;
}

void FakeCursor::setClickType(Click::but_t c)
{
	clickType = c;
}

bool FakeCursor::checkLimits(Point p)
{
	if (p.x < 0)
	{
		// A ZONE
		setClickType(Click::RIGHT);
		return false;	
	}
	if (p.y < 0)
	{
		// B ZONE
		setClickType(Click::DOUBLE);
		return false;
	}
	if (p.x > Scr::getScreenWidth())
	{
		// C ZONE
		setClickType(Click::NOCLICK);
		return false;
	}
	if (p.y > Scr::getScreenHeight())
	{
		// D ZONE
		setClickType(Click::NOCLICK);
		return false;
	}
	
	return true;
}

void FakeCursor::update()
{		
	if (!wii) return;
	if (state != ACTIVE) return;
	
	if (wii->dataReady())
	{
		if (!click)
			Filter::init();
		
		Point p = Filter::process(wii->getPos());
		
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
		if ((click) && (!click->refresh(false)))
		{
			clickType = Click::LEFT;
			delete click;
			click = 0;
// 			Filter::init();
		}
	}
	
}


void FakeCursor::move(Point p)
{
	display = XOpenDisplay(0);
	XTestFakeMotionEvent(display,-1,p.x,p.y,0);
	XCloseDisplay(display);
}




