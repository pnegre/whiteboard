
#ifndef _CURSOR_H_INCLUDED_
#define _CURSOR_H_INCLUDED_

#include "wiimote.h"

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>


class Click
{
	public:
		
	typedef enum { LEFT = 1, RIGHT=3, DOUBLE, NOCLICK } but_t;
	
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
	protected:
	
	Display *display;
	Wiimote *wii;
	
	typedef enum { ACTIVE, INACTIVE } state_t;
	state_t state;
	
	Click::but_t clickType;
	Click *click;

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
	
};



#endif