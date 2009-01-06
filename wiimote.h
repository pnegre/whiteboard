#ifndef _WIIMOTE_H_INCLUDED_
#define _WIIMOTE_H_INCLUDED_

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


#include "cwiid.h"
#include "common.h"


#define MAX_WII_X 1020
#define MAX_WII_Y 760


class Wiimote
{
	protected:

	typedef enum { DISCONNECTED, CONNECTED, CALIBRATED } state_t;
	state_t state;
	Point p;
	Point oldPoint;
	// Calibration data
	float h11,h12,h13,h21,h22,h23,h31,h32;
	
	int button;
	
	cwiid_wiimote_t *wiimote;
	
	int newData;
	
	void irData(Point &pt);
	void pressButton();
	
	void setLedState(cwiid_wiimote_t *wiimote, unsigned char led_state);
	void setRptMode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
	
	public:
		
	class Error {};
	class ErrorConnection: public Error {};
	class ErrorOther: public Error {};

	Wiimote();
	bool connection();
	bool getMsgs();
	bool endConnection();
	bool isButtonPressed();
	bool dataReady();
	void calibrate(Point p_screen[], Point p_wii[]);
	Point getPos();
	
};






#endif
