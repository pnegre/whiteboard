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









