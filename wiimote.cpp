


#include "wiimote.h"

extern "C" {
	#include "matrix.h"
}

#include "point.h"

#include "cwiid.h"



void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state)
{
	if (cwiid_command(wiimote, CWIID_CMD_LED, led_state)) {
		fprintf(stderr, "Error setting LEDs \n");
	}
}
	
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_command(wiimote, CWIID_CMD_RPT_MODE, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
};


Wiimote::Wiimote()
{
	state = DISCONNECTED;
	p.x = p.y = 0;
	button = 0;
	wiimote = 0;
	newData = 0;
}

bool Wiimote::connection()
{
	bdaddr_t bdaddr;
	bdaddr = *BDADDR_ANY;

	printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
	if (!(wiimote = cwiid_connect(&bdaddr, 0))) {
			fprintf(stderr, "Unable to connect to wiimote\n");
			return false;
	}
	printf("Connected!\n");
	
	set_led_state(wiimote, CWIID_LED1_ON);
	set_rpt_mode(wiimote, CWIID_RPT_IR | CWIID_RPT_BTN);
	cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC);
	cwiid_enable(wiimote, CWIID_FLAG_NONBLOCK);
	cwiid_disable(wiimote, CWIID_FLAG_CONTINUOUS);
	
	state = CONNECTED;
	return true;
}

bool Wiimote::getMsgs()
{
	int msg_count = 0;
	cwiid_mesg *mesg = 0;
	timespec tspec;
	cwiid_get_mesg(wiimote, &msg_count, &mesg, &tspec);
	//std::cout << "Msgs: " << msg_count << "\n";
	//std::cout << "Msgs: " << mesg << "\n";
	
	int valid_source = 0;
	int i,j;
	for (i=0; i < msg_count; i++)
	{
		switch (mesg[i].type) {
		case CWIID_MESG_BTN:
			//std::cout << "EE\n";
			printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
			pressButton();
			break;
		case CWIID_MESG_IR:
			//std::cout << "II\n";
			valid_source = 0;
			static int v[8];
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
				//std::cout << j << "\n";
				if (mesg[i].ir_mesg.src[j].valid) {
					valid_source = 1;
					//printf("(%d,%d) \n\n", mesg[i].ir_mesg.src[j].pos[CWIID_X],
					//                   mesg[i].ir_mesg.src[j].pos[CWIID_Y]);
					v[j*2] = mesg[i].ir_mesg.src[j].pos[CWIID_X];
					v[j*2+1] = mesg[i].ir_mesg.src[j].pos[CWIID_Y];
				}
				else v[j*2] = v[j*2+1] = 0;
			}
			if (!valid_source) {
				//printf("no sources detected\n");
			}
			else
			{
				irData(v);
				return true;
				//std::cout << "OO\n";
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

void Wiimote::irData(int *v)
{
	switch(state)
	{
		case (CONNECTED):
			p.x = v[0];
			p.y = v[1];
			break;
		
		case (CALIBRATED):
			p.x =  (int) ( ( (float)  (h11*v[0] + h12*v[1] + h13) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
			p.y =  (int) ( ( (float)  (h21*v[0] + h22*v[1] + h23) ) / ( (float) (h31*v[0] + h32*v[1] + 1) ) );
			break;
			
		default:
			break;
	}
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

