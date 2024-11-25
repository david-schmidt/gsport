/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2019 by GSport contributors
 Copyright (C) 2016 - 2018  Dagen Brock
 
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
#include "glog.h"

#ifdef __linux__
# include <linux/joystick.h>
# include <sys/time.h>
#endif

#ifdef _WIN32
# include <windows.h>
# include <mmsystem.h>
# include <time.h>
#endif

#ifdef HAVE_SDL
# include "SDL.h"
//@todo: multiple joysticks/more buttons/button config
//static SDL_Joystick *joy0, *joy1;
SDL_Joystick *gGameController = NULL;
#endif



extern int g_joystick_native_type1;             /* in paddles.c */
extern int g_joystick_native_type2;             /* in paddles.c */
extern int g_joystick_native_type;              /* in paddles.c */
extern int g_joystick_type;
extern int g_paddle_buttons;
extern int g_paddle_val[];


const char *g_joystick_dev = "/dev/input/js0";  /* default joystick dev file */
#define MAX_JOY_NAME    128

int g_joystick_native_fd = -1;
int g_joystick_num_axes = 0;
int g_joystick_num_buttons = 0;
int g_joystick_number = 0;                              // SDL2
int g_joystick_x_axis = 0;                              // SDL2
int g_joystick_y_axis = 1;                              // SDL2
int g_joystick_button_0 = 0;                    // SDL2
int g_joystick_button_1 = 1;            // SDL2

int g_joystick_x2_axis = 2;                             // SDL2
int g_joystick_y2_axis = 3;                             // SDL2
int g_joystick_button_2 = 2;                    // SDL2
int g_joystick_button_3 = 3;            // SDL2
#define JOY2SUPPORT


#if defined(HAVE_SDL) && !defined(JOYSTICK_DEFINED)
# define JOYSTICK_DEFINED
void joystick_init()      {
  int i;
  if( SDL_Init( SDL_INIT_JOYSTICK ) < 0 ) {
    glogf( "SDL could not initialize joystick! SDL Error: %s", SDL_GetError() );
  } else {
    glog("SDL2 joystick initialized");
  }
  if (SDL_NumJoysticks()<1) {
    glog("No joysticks detected");
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  } else {
    // @todo: make controller configurable
    // @todo: add multiple controller support
    gGameController = SDL_JoystickOpen( g_joystick_number );
    if( gGameController == NULL ) {
      glogf( "Warning: Unable to open game controller! SDL Error: %s", SDL_GetError() );
    }
  }
  g_joystick_native_type = 2;
  g_joystick_native_type1 = 2;
  g_joystick_native_type2 = -1;
  for(i = 0; i < 4; i++) {
    g_paddle_val[i] = 180;
  }
  g_joystick_type = JOYSTICK_TYPE_NATIVE_1;
  SDL_JoystickUpdate();
  joystick_update(0.0);
}

void joystick_update(double dcycs)      {
  if (gGameController) {
    SDL_JoystickUpdate();
    g_paddle_val[0] = (int)SDL_JoystickGetAxis(gGameController, g_joystick_x_axis);   // default is 0
    g_paddle_val[1] = (int)SDL_JoystickGetAxis(gGameController, g_joystick_y_axis);   // default is 1
    g_paddle_val[2] = (int)SDL_JoystickGetAxis(gGameController, g_joystick_x2_axis);  // default is 2
    g_paddle_val[3] = (int)SDL_JoystickGetAxis(gGameController, g_joystick_y2_axis);  // default is 3

    if (SDL_JoystickGetButton(gGameController, g_joystick_button_0)) {
      g_paddle_buttons = g_paddle_buttons | 1;
    } else {
      g_paddle_buttons = g_paddle_buttons & (~1);
    }
    if (SDL_JoystickGetButton(gGameController, g_joystick_button_1)) {
      g_paddle_buttons = g_paddle_buttons | 2;
    } else {
      g_paddle_buttons = g_paddle_buttons & (~2);
    }
    if (SDL_JoystickGetButton(gGameController, g_joystick_button_2)) {
      g_paddle_buttons = g_paddle_buttons | 4;
    } else {
      g_paddle_buttons = g_paddle_buttons & (~4);
    }
    if (SDL_JoystickGetButton(gGameController, g_joystick_button_3)) {
      g_paddle_buttons = g_paddle_buttons | 8;
    } else {
      g_paddle_buttons = g_paddle_buttons & (~8);
    }
    paddle_update_trigger_dcycs(dcycs);
  }
}

void joystick_update_buttons()      {
}

void joystick_shut() {
  SDL_JoystickClose( gGameController );
  gGameController = NULL;
}
#endif





#if defined(__linux__) && !defined(JOYSTICK_DEFINED)
# define JOYSTICK_DEFINED
void joystick_init()      {
  char joy_name[MAX_JOY_NAME];
  int version;
  int fd;
  int i;

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
void joystick_update(double dcycs)      {
  struct js_event js;           /* the linux joystick event record */
  int mask;
  int val;
  int num;
  int type;
  int ret;
  int len;
  int i;

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
    num = js.number & 3;                        /* clamp to 0-3 */
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

//	if(i > 0) {
//	Note from Dave Schmenk: paddle_update_trigger_dcycles(dcycs) always has to be called to keep the triggers current.
  paddle_update_trigger_dcycs(dcycs);
//	}
}

void joystick_update_buttons()      {
}
#endif /* LINUX */

#if defined(_WIN32) && !defined(JOYSTICK_DEFINED)
# define JOYSTICK_DEFINED
void joystick_init()      {
  JOYINFO info;
  JOYCAPS joycap;
  MMRESULT ret1, ret2;
  int i;

  //	Check that there is a joystick device
  if(joyGetNumDevs() <= 0) {
    glog("No joystick hardware detected");
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
    glog("No joystick is attached");
    return;
  }

  for(i = 0; i < 4; i++) {
    g_paddle_val[i] = 32767;
  }
  g_paddle_buttons = 0xc;

  joystick_update(0.0);
}

void joystick_update(double dcycs)      {
  JOYCAPS joycap;
  JOYINFO info;
  UINT id;
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

void joystick_update_buttons()      {
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
void joystick_init()      {
  g_joystick_native_type1 = -1;
  g_joystick_native_type2 = -1;
  g_joystick_native_type = -1;
}

void joystick_update(double dcycs)      {
  int i;

  for(i = 0; i < 4; i++) {
    g_paddle_val[i] = 32767;
  }
  g_paddle_buttons = 0xc;
}

void joystick_update_buttons()      {
}

void joystick_shut() {
}
#endif
