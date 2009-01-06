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

extern "C" {
	#include "matrix.h"
}

#include <iostream>





void Wiimote::setLedState(cwiid_wiimote_t *wiimote, unsigned char led_state)
{
	if (cwiid_command(wiimote, CWIID_CMD_LED, led_state)) {
		fprintf(stderr, "Error setting LEDs \n");
		throw ErrorOther();
	}
}
	
void Wiimote::setRptMode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_command(wiimote, CWIID_CMD_RPT_MODE, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
		throw ErrorOther();
	}
}


Wiimote::Wiimote()
{
	state = DISCONNECTED;
	p.x = p.y = 0;
	button = 0;
	wiimote = 0;
	newData = 0;
	oldPoint.x = oldPoint.y = 0;
}

bool Wiimote::connection()
{
	bdaddr_t bdaddr;
	bdaddr = *BDADDR_ANY;

	std::cout << "Put Wiimote in discoverable mode now (press 1+2)...\n";
	if (!(wiimote = cwiid_connect(&bdaddr, 0))) {
			throw (ErrorConnection());
	}
	std::cout << "Connected!\n";
	
	setLedState(wiimote, CWIID_LED1_ON);
	setRptMode(wiimote, CWIID_RPT_IR | CWIID_RPT_BTN);
	cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC);
	cwiid_disable(wiimote, CWIID_FLAG_NONBLOCK);
	cwiid_disable(wiimote, CWIID_FLAG_CONTINUOUS);
	
	state = CONNECTED;
	return true;
}


static int sqdist(Point &p, Point &q)
{
	return ((p.x-q.x)*(p.x-q.x) + (p.y-q.y)*(p.y-q.y));
}



bool Wiimote::getMsgs()
{
	int msg_count = 0;
	cwiid_mesg *mesg = 0;
	timespec tspec;
	cwiid_get_mesg(wiimote, &msg_count, &mesg, &tspec);
	
	int i,j;
	for (i=0; i < msg_count; i++)
	{
		switch (mesg[i].type) {
		case CWIID_MESG_BTN:
			printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
			pressButton();
			break;
		case CWIID_MESG_IR:
			static Point p;
			Point q;
			int validData;
			int dist;
			dist = -1;
			validData = 0;
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) 
			{
				if (mesg[i].ir_mesg.src[j].valid) 
				{
					validData = 1;
					q.x = mesg[i].ir_mesg.src[j].pos[CWIID_X];
					q.y = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
					
					int d = sqdist(q,oldPoint);
					if ((dist == -1) || (d < dist)) { p = q; dist = d; }
				}
			}
			if (validData)
			{
				irData(p);
				return true;
			}
			break;
		case CWIID_MESG_ERROR:
			if (cwiid_disconnect(wiimote)) {
				fprintf(stderr, "Error on wiimote disconnect\n");
				exit(-1);
			}
			exit(0);
			break;
		default:
			break;
		}
	}
	
	if (msg_count)
		return true;	
}

bool Wiimote::endConnection()
{
	cwiid_disconnect(wiimote);
	state = DISCONNECTED;
}

bool Wiimote::isButtonPressed()
{
	return (button == 1);
}

bool Wiimote::dataReady()
{
	if (newData)
	{
		newData = 0;
		return true;
	}
	return false;
}

void Wiimote::irData(Point &pt)
{
	switch(state)
	{
		case (CONNECTED):
			p = pt;
			break;
		
		case (CALIBRATED):
			p.x =  (int) ( ( (float)  (h11*pt.x + h12*pt.y + h13) ) / 
				( (float) (h31*pt.x + h32*pt.y + 1) ) );
			p.y =  (int) ( ( (float)  (h21*pt.x + h22*pt.y + h23) ) / 
				( (float) (h31*pt.x + h32*pt.y + 1) ) );
			break;
			
		default:
			break;
	}
	
	oldPoint = pt;
	newData = 1;
}

void Wiimote::calibrate(Point p_screen[], Point p_wii[])
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

	h11 = matrixGetElement(r,0,0);
	h12 = matrixGetElement(r,0,1);
	h13 = matrixGetElement(r,0,2);
	h21 = matrixGetElement(r,0,3);
	h22 = matrixGetElement(r,0,4);
	h23 = matrixGetElement(r,0,5);
	h31 = matrixGetElement(r,0,6);
	h32 = matrixGetElement(r,0,7);
	
	matrixFree(m);
	matrixFree(n);
	matrixFree(r);
	
	state = CALIBRATED;
}

Point Wiimote::getPos()
{
	return p;
}

void Wiimote::pressButton()
{
	button = 1;
}

