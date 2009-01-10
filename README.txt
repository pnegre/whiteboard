Linux Electronic Whiteboard with Wiimote
----------------------------------------

This is my "working version" of linux whiteboard (see
http://code.google.com/p/linux-whiteboard/)

It uses SDL for calibration, no GUI.

To build, use scons:

	$ scons
	[ .. it will search for necessary libraries ..]

No need to ./configure

To run:

	$ whiteboard

Make sure that you have bluetooth working. Tested in Debian testing and Ubuntu
Intrepid Ibex.
