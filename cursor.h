#ifndef _CURSOR_H_INCLUDED_
#define _CURSOR_H_INCLUDED_

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



#include "wiimote.h"

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>


class Click
{
	public:
		
	typedef enum { LEFT, RIGHT, DOUBLE, NOCLICK } but_t;
	
	protected:
	
	but_t but;
	int initialTime;
	
	public:
	
	Click(but_t b);
	bool refresh(bool evt);
	void button(bool press);
	
	
	
};


class FakeCursor
{
	public:
		
	typedef enum { ZONE_A, ZONE_B, ZONE_C, ZONE_D } zone_t;
	
	protected:
	
	Display *display;
	Wiimote *wii;
	
	typedef enum { ACTIVE, INACTIVE } state_t;
	state_t state;
	
	Click::but_t clickType;
	Click *click;
	
	Click::but_t zoneAClick, zoneBClick, zoneCClick, zoneDClick;
	
	public:
	
	FakeCursor();
	void attachWiimote(Wiimote *wiim);
	Wiimote *getWii();
	void activate();
	void deactivate();
	void setClickType(Click::but_t c);
	bool checkLimits(Point p);
	void update();
	void move(Point p);
	void configureLimit(zone_t z, Click::but_t c);
	
};



#endif