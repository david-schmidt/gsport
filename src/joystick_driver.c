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

const char rcsid_joystick_driver_c[] = "@(#)$KmKId: joystick_driver.c,v 1.12 2004-10-17 21:28:48-04 kentd Exp $";

#include "defc.h"
#include <sys/time.h>

#ifdef __linux__
# include <linux/joystick.h>
#endif

#ifdef _WIN32
# include <windows.h>
# include <mmsystem.h>
#endif

extern int g_joystick_native_type1;		/* in paddles.c */
extern int g_joystick_native_type2;		/* in paddles.c */
extern int g_joystick_native_type;		/* in paddles.c */
extern int g_paddle_buttons;
extern int g_paddle_val[];


const char *g_joystick_dev = "/dev/js0";	/* default joystick dev file */
#define MAX_JOY_NAME	128

int	g_joystick_native_fd = -1;
int	g_joystick_num_axes = 0;
int	g_joystick_num_buttons = 0;


#ifdef __linux__
# define JOYSTICK_DEFINED
void
joystick_init()
{
	char	joy_name[MAX_JOY_NAME];
	int	version;
	int	fd;
	int	i;

	fd = open(g_joystick_dev, O_RDONLY | O_NONBLOCK);
	if(fd < 0) {
		printf("Unable to open joystick dev file: %s, errno: %d\n",
			g_joystick_dev, errno);
		printf("Defaulting to mouse joystick\n");
		return;
	}

	strcpy(&joy_name[0], "Unknown Joystick");
	version = 0x800;

	ioctl(fd, JSIOCGNAME(MAX_JOY_NAME), &joy_name[0]);
	ioctl(fd, JSIOCGAXES, &g_joystick_num_axes);
	ioctl(fd, JSIOCGBUTTONS, &g_joystick_num_buttons);
	ioctl(fd, JSIOCGVERSION, &version);

	printf("Detected joystick: %s [%d axes, %d buttons vers: %08x]\n",
		joy_name, g_joystick_num_axes, g_joystick_num_buttons,
		version);

	g_joystick_native_type1 = 1;
	g_joystick_native_type2 = -1;
	g_joystick_native_fd = fd;
	for(i = 0; i < 4; i++) {
		g_paddle_val[i] = 32767;
	}
	g_paddle_buttons = 0xc;

	joystick_update(0.0);
}

/* joystick_update_linux() called from paddles.c.  Update g_paddle_val[] */
/*  and g_paddle_buttons with current information */
void
joystick_update(double dcycs)
{
	struct js_event js;	/* the linux joystick event record */
	int	mask;
	int	val;
	int	num;
	int	type;
	int	ret;
	int	len;
	int	i;

	/* suck up to 20 events, then give up */
	len = sizeof(struct js_event);
	for(i = 0; i < 20; i++) {
		ret = read(g_joystick_native_fd, &js, len);
		if(ret != len) {
			/* just get out */
			break;
		}
		type = js.type & ~JS_EVENT_INIT;
		val = js.value;
		num = js.number & 3;		/* clamp to 0-3 */
		switch(type) {
		case JS_EVENT_BUTTON:
			mask = 1 << num;
			if(val) {
				val = mask;
			}
			g_paddle_buttons = (g_paddle_buttons & ~mask) | val;
			break;
		case JS_EVENT_AXIS:
			/* val is -32767 to +32767 */
			g_paddle_val[num] = val;
			break;
		}
	}

	if(i > 0) {
		paddle_update_trigger_dcycs(dcycs);
	}
}

void
joystick_update_buttons()
{
}
#endif /* LINUX */

#ifdef _WIN32
# define JOYSTICK_DEFINED
void
joystick_init()
{
	JOYINFO info;
	JOYCAPS joycap;
	MMRESULT ret1, ret2;
	int	i;

	//	Check that there is a joystick device
	if(joyGetNumDevs() <= 0) {
		printf("No joystick hardware detected\n");
		g_joystick_native_type1 = -1;
		g_joystick_native_type2 = -1;
		return;
	}

	g_joystick_native_type1 = -1;
	g_joystick_native_type2 = -1;

	//	Check that at least joystick 1 or joystick 2 is available
	ret1 = joyGetPos(JOYSTICKID1, &info);
	ret2 = joyGetDevCaps(JOYSTICKID1, &joycap, sizeof(joycap));
	if(ret1 == JOYERR_NOERROR && ret2 == JOYERR_NOERROR) {
		g_joystick_native_type1 = JOYSTICKID1;
		printf("Joystick #1 = %s\n", joycap.szPname);
		g_joystick_native_type = JOYSTICKID1;
	}
	ret1 = joyGetPos(JOYSTICKID2, &info);
	ret2 = joyGetDevCaps(JOYSTICKID2, &joycap, sizeof(joycap));
	if(ret1 == JOYERR_NOERROR && ret2 == JOYERR_NOERROR) {
		g_joystick_native_type2 = JOYSTICKID2;
		printf("Joystick #2 = %s\n", joycap.szPname);
		if(g_joystick_native_type < 0) {
			g_joystick_native_type = JOYSTICKID2;
		}
	}

	if (g_joystick_native_type1<0 && g_joystick_native_type2 <0) {
		printf ("No joystick is attached\n");
		return;
	}

	for(i = 0; i < 4; i++) {
		g_paddle_val[i] = 32767;
	}
	g_paddle_buttons = 0xc;

	joystick_update(0.0);
}

void
joystick_update(double dcycs)
{
	JOYCAPS joycap;
	JOYINFO info;
	UINT	id;
	MMRESULT ret1, ret2;

	id = g_joystick_native_type;

	ret1 = joyGetDevCaps(id, &joycap, sizeof(joycap));
	ret2 = joyGetPos(id, &info);
	if(ret1 == JOYERR_NOERROR && ret2 == JOYERR_NOERROR) {
		g_paddle_val[0] = (info.wXpos - joycap.wXmin) * 32768 /
						(joycap.wXmax - joycap.wXmin);
		g_paddle_val[1] = (info.wYpos - joycap.wYmin) * 32768 /
						(joycap.wYmax - joycap.wYmin);
		if(info.wButtons & JOY_BUTTON1) {
			g_paddle_buttons = g_paddle_buttons | 1;
		} else {
			g_paddle_buttons = g_paddle_buttons & (~1);
		}
		if(info.wButtons & JOY_BUTTON2) {
			g_paddle_buttons = g_paddle_buttons | 2;
		} else {
			g_paddle_buttons = g_paddle_buttons & (~2);
		}
		paddle_update_trigger_dcycs(dcycs);
	}
}

void
joystick_update_buttons()
{
	JOYINFOEX info;
	UINT id;

	id = g_joystick_native_type;

	info.dwSize = sizeof(JOYINFOEX);
	info.dwFlags = JOY_RETURNBUTTONS;
	if(joyGetPosEx(id, &info) == JOYERR_NOERROR) {
		if(info.dwButtons & JOY_BUTTON1) {
			g_paddle_buttons = g_paddle_buttons | 1;
		} else {
			g_paddle_buttons = g_paddle_buttons & (~1);
		}
		if(info.dwButtons & JOY_BUTTON2) {
			g_paddle_buttons = g_paddle_buttons | 2;
		} else {
			g_paddle_buttons = g_paddle_buttons & (~2);
		}
	}
}
#endif

#ifndef JOYSTICK_DEFINED
/* stubs for the routines */
void
joystick_init()
{
	g_joystick_native_type1 = -1;
	g_joystick_native_type2 = -1;
	g_joystick_native_type = -1;
}

void
joystick_update(double dcycs)
{
	int	i;

	for(i = 0; i < 4; i++) {
		g_paddle_val[i] = 32767;
	}
	g_paddle_buttons = 0xc;
}

void
joystick_update_buttons()
{
}
#endif
