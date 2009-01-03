#ifndef _WIIMOTE_H_INCLUDED_
#define _WIIMOTE_H_INCLUDED_


#include "cwiid.h"
#include "point.h"


#define MAX_WII_X 1020
#define MAX_WII_Y 760


class Wiimote
{
	protected:

	typedef enum { DISCONNECTED, CONNECTED, CALIBRATED } state_t;
	state_t state;
	Point p;
	// Calibration data
	float h11,h12,h13,h21,h22,h23,h31,h32;
	
	int button;
	
	cwiid_wiimote_t *wiimote;
	struct cwiid_state wiiState;
	
	int newData;

	public:

	Wiimote();
	bool connection();
	bool getMsgs();
	bool endConnection();
	bool isButtonPressed();
	bool dataReady();
	void irData(int *v);
	void calibrate(Point p_screen[], Point p_wii[]);
	Point getPos();
	void pressButton();
};






#endif