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


#include <stdlib.h>
#include <iostream>


#include "calibration.h"
#include "wiimote.h"
#include "timer.h"
#include "cursor.h"



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



