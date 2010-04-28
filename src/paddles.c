/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 by GSport contributors
 
 Based on the KEGS emulator written by and Copyright (C) 2003 Kent Dickey

 This program is free software; you can redistribute it and/or modify it 
 under the terms of the GNU General Public License as published by the 
 Free Software Foundation; either version 2 of the License, or (at your 
 option) any later version.

 This program is distributed in the hope that it will be useful, but 
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 for more details.

 You should have received a copy of the GNU General Public License along 
 with this program; if not, write to the Free Software Foundation, Inc., 
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "defc.h"

extern int g_mouse_raw_x;	/* from adb.c */
extern int g_mouse_raw_y;

double	g_paddle_trig_dcycs = 0.0;
int	g_swap_paddles = 0;
int	g_invert_paddles = 0;
int	g_joystick_scale_factor_x = 0x100;
int	g_joystick_scale_factor_y = 0x100;
int	g_joystick_trim_amount_x = 0;
int	g_joystick_trim_amount_y = 0;

int	g_joystick_type = 0;	/* 0 = Keypad Joystick */
int	g_joystick_native_type1 = -1;
int	g_joystick_native_type2 = -1;
int	g_joystick_native_type = -1;

extern int g_paddle_buttons;

int	g_paddle_val[4] = { 0, 0, 0, 0 };
		/* g_paddle_val[0]: Joystick X coord, [1]:Y coord */

double	g_paddle_dcycs[4] = { 0.0, 0.0, 0.0, 0.0 };
		/* g_paddle_dcycs are the dcycs the paddle goes to 0 */


void
paddle_fixup_joystick_type()
{
	/* If g_joystick_type points to an illegal value, change it */
	if(g_joystick_type == 2) {
		g_joystick_native_type = g_joystick_native_type1;
		if(g_joystick_native_type1 < 0) {
			g_joystick_type = 0;
		}
	}
	if(g_joystick_type == 3) {
		g_joystick_native_type = g_joystick_native_type2;
		if(g_joystick_native_type2 < 0) {
			g_joystick_type = 0;
		}
	}
}

void
paddle_trigger(double dcycs)
{
	/* Called by read/write to $c070 */
	g_paddle_trig_dcycs = dcycs;

	/* Determine what all the paddle values are right now */
	paddle_fixup_joystick_type();

	switch(g_joystick_type) {
	case 0:		/* Keypad Joystick */
		paddle_trigger_keypad(dcycs);
		break;
	case 1:		/* Mouse Joystick */
		paddle_trigger_mouse(dcycs);
		break;
	default:
		joystick_update(dcycs);
	}
}

void
paddle_trigger_mouse(double dcycs)
{
	int	val_x, val_y;
	int	mouse_x, mouse_y;

	val_x = 0;

	mouse_x = g_mouse_raw_x;
	mouse_y = g_mouse_raw_y;
	/* mous_phys_x is 0->560, convert that to -32768 to + 32767 cyc */
	/*  So subtract 280 then mult by 117 */
	val_x = (mouse_x - 280) * 117;

	/* mous_phys_y is 0->384, convert that to -32768 to + 32767 cyc */
	/*  so subtract 192 then mult by 180 to overscale it a bit */
	val_y = (mouse_y - 192) * 180;

	g_paddle_val[0] = val_x;
	g_paddle_val[1] = val_y;
	g_paddle_val[2] = 32767;
	g_paddle_val[3] = 32767;
	g_paddle_buttons |= 0xc;
	paddle_update_trigger_dcycs(dcycs);
}

void
paddle_trigger_keypad(double dcycs)
{
	int	get_y;
	int	val_x, val_y;


	val_x = adb_get_keypad_xy(get_y=0);
	val_y = adb_get_keypad_xy(get_y=1);
	/* val_x and val_y are already scale -32768 to +32768 */

	g_paddle_val[0] = val_x;
	g_paddle_val[1] = val_y;
	g_paddle_val[2] = 32767;
	g_paddle_val[3] = 32767;
	g_paddle_buttons |= 0xc;
	paddle_update_trigger_dcycs(dcycs);
}

void
paddle_update_trigger_dcycs(double dcycs)
{
	double	trig_dcycs;
	int	val;
	int	paddle_num;
	int	scale, trim;
	int	i;

	for(i = 0; i < 4; i++) {
		paddle_num = i;
		if(g_swap_paddles) {
			paddle_num = i ^ 1;
		}
		val = g_paddle_val[paddle_num];
		if(g_invert_paddles) {
			val = -val;
		}
		/* convert -32768 to +32768 into 0->2816.0 cycles (the */
		/* paddle delay const) */
		/* First multiply by the scale factor to adjust range */
		if(paddle_num & 1) {
			scale = g_joystick_scale_factor_y;
			trim = g_joystick_trim_amount_y;
		} else {
			scale = g_joystick_scale_factor_x;
			trim = g_joystick_trim_amount_x;
		}
#if 0
		if(i == 0) {
			printf("val was %04x(%d) * scale %03x = %d\n",
				val, val, scale, (val*scale)>>16);
		}
#endif
		val = val * scale >> 16;
		/* Val is now from -128 to + 128 since scale is */
		/*  256=1.0, 128 = 0.5 */
		val = val + 128 + trim;
		if(val >= 255) {
			val = 280;	/* increase range */
		}
		trig_dcycs = dcycs + (val * 11.04);
		g_paddle_dcycs[i] = trig_dcycs;
	}
}

int
read_paddles(double dcycs, int paddle)
{
	double	trig_dcycs;

	trig_dcycs = g_paddle_dcycs[paddle & 3];

	if(dcycs < trig_dcycs) {
		return 0x80;
	} else {
		return 0x00;
	}
}

void
paddle_update_buttons()
{
	paddle_fixup_joystick_type();
	joystick_update_buttons();
}
