/* Copyright (C) 2007 L. Donnie Smith <cwiid@abstrakraft.org>
 * 
 *  (modified by Pere Negre)
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




#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <cwiid.h>

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count, union cwiid_mesg mesg[]);

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

void set_led_state(cwiid_wiimote_t *wiimote, unsigned char led_state);
void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode);
void print_state(struct cwiid_state *state);

cwiid_err_t err;
void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}

//////////////////////////////////////////


cwiid_wiimote_t *wiimote;       /* wiimote handle */
struct cwiid_state state;       /* wiimote state */


extern void infrared_data(int *v);
extern void buttonpress();
extern void update_cursor();

int wii_connect(char *mac)
{
	bdaddr_t bdaddr;        /* bluetooth device address */
	unsigned char rpt_mode = 0;

	cwiid_set_err(err);

	str2ba(mac, &bdaddr);
	
	if (mac[0]!='#') {
		str2ba(mac, &bdaddr);
	}
	else {
		bdaddr = *BDADDR_ANY;
	}

	/* Connect to the wiimote */
        printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
        if (!(wiimote = cwiid_connect(&bdaddr, 0))) {
                fprintf(stderr, "Unable to connect to wiimote\n");
                return 0;
        }
	printf("Connected!!!\n");
        if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
                fprintf(stderr, "Unable to set message callback\n");
		return 0;
        }

	set_led_state(wiimote,1);

        toggle_bit(rpt_mode, CWIID_RPT_IR);
	toggle_bit(rpt_mode, CWIID_RPT_BTN);
        set_rpt_mode(wiimote, rpt_mode);
        cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC);
	return 1;
}




void wii_disconnect()
{
	set_led_state(wiimote,0);

	if (cwiid_disconnect(wiimote)) {
                fprintf(stderr, "Error on wiimote disconnect\n");
        }
}


////////////////////////////////////////////




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
}


/* Prototype cwiid_callback with cwiid_callback_t, define it with the actual
 * type - this will cause a compile error (rather than some undefined bizarre
 * behavior) if cwiid_callback_t changes */
/* cwiid_mesg_callback_t has undergone a few changes lately, hopefully this
 * will be the last.  Some programs need to know which messages were received
 * simultaneously (e.g. for correlating accelerometer and IR data), and the
 * sequence number mechanism used previously proved cumbersome, so we just
 * pass an array of messages, all of which were received at the same time.
 * The id is to distinguish between multiple wiimotes using the same callback.
 * */
void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg[])
{
	int i, j;
	int valid_source;

	for (i=0; i < mesg_count; i++)
	{
		switch (mesg[i].type) {
		case CWIID_MESG_BTN:
			printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
			buttonpress();
			break;
		case CWIID_MESG_IR:
			valid_source = 0;
			static int v[8];
			for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
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
				//printf("no sources detected");
			}
			else
				infrared_data(v);

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
	update_cursor();
}

