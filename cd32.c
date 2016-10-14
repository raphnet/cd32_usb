/* cd32_to_usb: CD32 controller to USB adapter
 * Copyright (C) 2009 Raphaël Assénat
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The author may be contacted at raph@raphnet.net
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "gamepad.h"
#include "cd32.h"

#define REPORT_SIZE		3



// report matching the most recent bytes from the controller
static unsigned char last_read_controller_bytes[REPORT_SIZE];

// the most recently reported bytes
static unsigned char last_reported_controller_bytes[REPORT_SIZE];

typedef struct {
	char x,y;
	char btnRed, btnBlue, btnGreen, btnYellow;
	char btnStart, btnL, btnR, btnXtra;
} JagCtlState;

static void readController(JagCtlState *st)
{
	unsigned char btns[8];
	int i;

	memset(st, 0, sizeof(JagCtlState));
	st->x = st->y = 0x80;

	
	// first, do the easy directional buttons
	if (!(PINC & 0x20)) // Up
		st->y = 0;
	if (!(PINC & 0x10)) // Down
		st->y = 0xff;
	if (!(PINC & 0x08)) // Left
		st->x = 0;
	if (!(PINC & 0x04)) // Right
		st->x = 0xff;
	
	// Load
	PORTB |= 0x20;
	_delay_us(300);
	PORTB &= ~0x20;
	_delay_us(100);
	
	DDRC |= 0x02;
	_delay_us(100);
		
	for (i=0; i<8; i++) {
		

		btns[i] = !(PINC & 0x01);

		PORTC |= 0x02;
		_delay_us(300);

		PORTC &= ~(0x02);
		_delay_us(300);	
	}

	DDRC &=0x02;

	st->btnBlue = btns[0];
	st->btnRed = btns[1];	
	st->btnYellow = btns[2];
	st->btnGreen = btns[3];
	st->btnR = btns[4];
	st->btnL = btns[5];
	st->btnStart = btns[6];
	st->btnXtra = btns[7];
}



static void cd32Update(void)
{
	JagCtlState st;
	unsigned char btns1=0;

	readController(&st);

	if (st.btnRed)
		btns1 |= 0x01;
	if (st.btnBlue)
		btns1 |= 0x02;
	if (st.btnGreen)
		btns1 |= 0x04;
	if (st.btnYellow)
		btns1 |= 0x08;
	if (st.btnStart)
		btns1 |= 0x10;
	if (st.btnL)
		btns1 |= 0x20;
	if (st.btnR)
		btns1 |= 0x40;
	if (st.btnXtra)
		btns1 |= 0x80;

	last_read_controller_bytes[0]=st.x;
	last_read_controller_bytes[1]=st.y;
 	last_read_controller_bytes[2]=btns1;
}	

static void cd32Init(void)
{
	unsigned char sreg;
	sreg = SREG;
	cli();


	/* 
	 * IO  | DB9 pin | Color      | Description
	 *
	 * PC5 |  1       | White      | Up
	 * PC4 |  2       | Blue       | Down
	 * PC3 |  3       | Green      | Left
	 * PC2 |  4       | Brown      | Right
	 * PC1 |  6       | Orange     | Clock (to controller)
	 * PC0 |  9       | Grey       | Serial Data (from controller)
	 
	 * PB5 |  5       | Yellow     | Shift/Load  (When high, LOAD)
	 *
	 * VCC |  7       | Red        | Controller VCC
	 * GND |  8       | Black      | Controller GND
	 */

	// Clock output low
	DDRC=0x02;
	PORTC=0xfd; 

//	DDRC = 0x00;
//	PORTC = 0xff;

	DDRB=0x20;
	PORTB=0xff;

	cd32Update();

	SREG = sreg;
}
static char cd32Changed(void)
{
	static int first = 1;
	if (first) { first = 0;  return 1; }
	
	return memcmp(last_read_controller_bytes, 
					last_reported_controller_bytes, REPORT_SIZE);
}

static void cd32BuildReport(unsigned char *reportBuffer)
{
	if (reportBuffer != NULL)
	{
		memcpy(reportBuffer, last_read_controller_bytes, REPORT_SIZE);
	}
	memcpy(last_reported_controller_bytes, 
			last_read_controller_bytes, 
			REPORT_SIZE);	
}

static const char snes_usbHidReportDescriptor[] PROGMEM = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x02,                    //   REPORT_COUNT (2)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0xc0,                          // END_COLLECTION
    0x05, 0x09,                    // USAGE_PAGE (Button)
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, 0x07,                    //   USAGE_MAXIMUM (Button 7)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    // REPORT_SIZE (1)
    0x95, 0x07,                    // REPORT_COUNT (7)
    0x81, 0x02,                    // INPUT (Data,Var,Abs)

	/* Padding.*/
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x03,                    //     INPUT (Constant,Var,Abs)

    0xc0                           // END_COLLECTION
};


Gamepad cd32Gamepad = {
	report_size: 		REPORT_SIZE,
	reportDescriptorSize:	sizeof(snes_usbHidReportDescriptor),
	init: 			cd32Init,
	update: 		cd32Update,
	changed:		cd32Changed,
	buildReport:		cd32BuildReport
};

Gamepad *cd32GetGamepad(void)
{
	cd32Gamepad.reportDescriptor = (void*)snes_usbHidReportDescriptor;

	return &cd32Gamepad;
}

