Linux Electronic Whiteboard with Wiimote
----------------------------------------

This is my "working version" of linux whiteboard.

See http://code.google.com/p/linux-whiteboard/

It uses SDL for calibration, no GUI.

To build, you'll need a program named "scons". You can install it from your distibution's repositories.

Then, just type "scons":

	$ scons
	[ .. it will search for necessary libraries ..]

Maybe you'll need supplementary libraries. You can install them also from the official repositories.

No need to ./configure

To run:

	$ whiteboard

Make sure that you have bluetooth working. Tested in Debian Lenny and Ubuntu
Intrepid Ibex.

You can also install the program (do it as root):

	$ scons prefix=/ install

Use the prefix parameter to indicate the installation path. For prefix=/usr, the binary will be copied to /usr/bin.




