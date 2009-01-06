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

#include "common.h"

#include "calibration.h"
#include "wiimote.h"
#include "cursor.h"



int main(int argc,char *argv[])
{
	Timer::start();
	
	try
	{
		Wiimote wiim;
		wiim.connection();
		
		Calibration::calibrate(wiim);
		
		FakeCursor cursor;
		cursor.configureLimit(FakeCursor::ZONE_A, Click::RIGHT);
		cursor.configureLimit(FakeCursor::ZONE_B, Click::DOUBLE);
		cursor.configureLimit(FakeCursor::ZONE_D, Click::NOCLICK);
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
	catch (Wiimote::ErrorConnection)
	{
		std::cout << "There was a problem connecting to the wiimote(s)\n";
		return 1;
	}
	catch (Calibration::Error)
	{
		std::cout << "Error (or abort) during calibration\n";
		return 1;
	}
}



