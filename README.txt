Linux Electronic Whiteboard with Wiimote
----------------------------------------

If using Ubuntu 7.10 (gutsy gibbon):

    * First, make sure bluetooth is working
    * Extract the file "demo" (no need for a configuration file now)
    * sudo apt-get install lswm libcwiid0 libsdl1.2debian libxext6 libxtst6
    * sudo ln -s /usr/lib/libXtst.so.6 /usr/lib/libXtst.so
    * Now we are ready to execute it: ./demo
    * Put the wiimote in discovery mode (press 1+2)
    * If the computer cannot connect with the wiimote, you can try to pass the 
      wiimote address as the first argument: "./demo thewiimoteaddress". 
      To obtain this address, execute "lswm", as you press 1+2 in the wiimote.
    * When the wiimote is connected, you should see four crosses and small 
      rectangle at the center of the screen
    * Point the wiimote to the screen, and start moving your IR pen.
    * The position of the IR pen will show in the little square at the center 
      of the screen. You can use it to adjust the wiimote so the distance between
      the points at the corners is maximized.
    * With the IR pen, click at the crosses, in this order: top-left, top-right, 
      bottom-left, bottom-right.
    * After each click on the each cross, you have to press spacebar (you will 
      notice that the cross you have to click is blinking)
    * When you press the spacebar the fourth time, the IR pen and the mouse pointer 
      will be in sync. You can start to use your electronic whiteboard
    * To exit, press any wiimote's key.

If that version does not work for you, you can try earlier releases here.

If you want to compile it yourself, check the project in google code:
	http://linux-whiteboard.google.com

If you have subversion installed on your system, you can get a 
working copy just typing:

$ svn checkout http://linux-whiteboard.googlecode.com/svn/trunk/ linux-whiteboard-read-only

You need to install some development libraries:

$ sudo apt-get install libcwiid0-dev libsdl1.2-dev libxext-dev libbluetooth-dev

Maybe you will need to make a link for the libXTst library:

$ sudo ln -s /usr/lib/libXtst.so.6 /usr/lib/libXtst.so

(yes, I know that this readme needs improvement!)

GOOD LUCK, and happy hacking (linus dixit)
