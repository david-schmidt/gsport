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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "options.h"
#include "glog.h"
#include "defc.h"

// config is parsed in config.c :: config_parse_config_gsport_file()
// cli is parsed here.  would be nice to reuse some code

// Halts on bad reads.  Sets flags via engine_s.s:set_halt_act() function
extern int g_halt_on_bad_read;        // defined in sim65816.c
// Ignore bad memory accesses.
extern int g_ignore_bad_acc;          // defined in sim65816.c
// Ignore red alert halts.
extern int g_ignore_halts;            // defined in sim65816.c
// Size of RAM memory expansion card in bytes (default = 8*1024*1024 = 8MB)
extern int g_mem_size_exp;            // defined in sim65816.c
// Implemented in display drivers (not SDL2 though) ?
extern int g_screen_redraw_skip_amt;  // defined in video.c, implemented in various driver files

// Using simple dhires color map
extern int g_use_dhr140;              // defined in video.c
// Force B/W hires modes
extern int g_use_bw_hires;            // defined in video.c
// Set starting X/Y positions
extern int g_startx;                  // defined in video.c
extern int g_starty;                  // defined in video.c
extern int g_startw;                  // defined in video.c
extern int g_starth;                  // defined in video.c
// Use High DPI (Retina) display - SDL2
extern int g_highdpi;                 // defined in video.c
// Create borderless window - SDL2
extern int g_borderless;              // defined in video.c
// Allow window resizing, dragging to scale - SDL2
extern int g_resizeable;              // defined in video.c
// Allow the window scaling to be free from aspect contraints - SDL2
extern int g_noaspect;
// Don't explicitly set vsync present flag on renderer - SDL2
extern int g_novsync;              // defined in video.c
// Don't explicitly set HW accelerator flag on renderer - SDL2
extern int g_nohwaccel;              // defined in video.c
// Use SDL_WINDOW_FULLSCREEN_DESKTOP for fullscreen instead of switching modes
extern int g_fullscreen_desktop;
// Enable Dagen's scanline simulator (SDL2)
extern int g_scanline_simulator;      // defined in sim65816.c
// Ethernet (interface?)
extern int g_ethernet;                // defined in sim65816.c
// Enable and set port for Dagen's debugger
extern int g_dbg_enable_port;         // defined in debug.c
// Set preferred audio frequency
extern int g_preferred_rate;          // defined in sound_driver.c, implemented in various driver files
// Enable/disable audio
extern int g_audio_enable;            // defined in sound.c
// Start in fullscreen mode
extern int g_fullscreen;              // defined in adb.c, because weird driver writing for x

// Specify the joystick - SDL2
extern int g_joystick_number;         // defined in joystick_driver.c
extern int g_joystick_x_axis;         // defined in joystick_driver.c
extern int g_joystick_y_axis;         // defined in joystick_driver.c
extern int g_joystick_x2_axis;        // defined in joystick_driver.c
extern int g_joystick_y2_axis;        // defined in joystick_driver.c
extern int g_joystick_button_0;       // defined in joystick_driver.c
extern int g_joystick_button_1;       // defined in joystick_driver.c
extern int g_joystick_button_2;       // defined in joystick_driver.c
extern int g_joystick_button_3;       // defined in joystick_driver.c


// DEPRECATED: force bit depth (15/16/24) for X-Windows, might still work.
extern int g_force_depth;             // defined in sim65816.c
// DEPRECATED: Use X shared memory (MIT-SHM)
extern int g_use_shmem;               // defined in all the various drivers
// DEPRECATED: Set DISPLAY environment variable for X-Windows
extern char g_display_env[512];       // defined in sim65816.c
// DEPRECATED: Set VERBOSE flags for one or more subsystems as defined below
extern int Verbose;                   // defined in sim65816.c
// #define VERBOSE_DISK	0x001
// #define VERBOSE_IRQ	0x002
// #define VERBOSE_CLK	0x004
// #define VERBOSE_SHADOW	0x008
// #define VERBOSE_IWM	0x010
// #define VERBOSE_DOC	0x020
// #define VERBOSE_ADB	0x040
// #define VERBOSE_SCC	0x080
// #define VERBOSE_TEST	0x100
// #define VERBOSE_VIDEO	0x200
// #define VERBOSE_MAC
// This is deprecated because it is not well-defined or supported
// It should still work and some sort of system should be put in place
// to extend and fix this, or take it out.


extern const char *g_config_gsport_name_list[];
extern char g_config_gsport_screenshot_dir[];
extern char	*final_arg;

static const char parse_log_prefix[] = "Option set [CLI]:";

// this is here because we need to flip a bit to force B/W modes
extern int g_cur_a2_stat;

void help_exit();       // displays the cli help text and exits with 1

int parse_int(const char *str1, int min, int max)
{
  int tmp;
  tmp = strtol(str1, 0, 0);
  if (tmp > max) { tmp = max; }
  if (tmp < min) { tmp = min; }
  return tmp;
}
void parse_cli_options(int argc, char **argv) {
  int	i;
  int tmp1;
  int	skip_amt;
  char	*final_arg = 0;


  for(i = 1; i < argc; i++) {
    if( (!strcmp("-?", argv[i])) || (!strcmp("-h", argv[i])) || (!strcmp("-help", argv[i]))) {
      help_exit();
    } else if(!strcmp("-badrd", argv[i])) {
      glogf("%s Halting on bad reads", parse_log_prefix);
      g_halt_on_bad_read = 2;
    } else if(!strcmp("-fullscreen", argv[i])) {
      glogf("%s Starting emulator in fullscreen", parse_log_prefix);
      g_fullscreen = 1;
    } else if(!strcmp("-highdpi", argv[i])) {
      glogf("%s Creating window in High DPI mode", parse_log_prefix);
      g_highdpi = 1;
    } else if(!strcmp("-borderless", argv[i])) {
      glogf("%s Creating borderless window", parse_log_prefix);
      g_borderless = 1;
    } else if(!strcmp("-resizeable", argv[i])) {
      glogf("%s Window will be resizeable", parse_log_prefix);
      g_resizeable = 1;
    } else if(!strcmp("-noaspect", argv[i])) {
      glogf("%s Window will scale freely, without locking the aspect ratio", parse_log_prefix);
      g_noaspect = 1;
    } else if(!strcmp("-novsync", argv[i])) {
      glogf("%s Renderer skipping vsync flag", parse_log_prefix);
      g_novsync = 1;
    } else if(!strcmp("-nohwaccel", argv[i])) {
      glogf("%s Renderer skipping HW accel flag", parse_log_prefix);
      g_nohwaccel = 1;
    } else if(!strcmp("-fulldesk", argv[i])) {
      glogf("%s Using desktop fullscreen mode", parse_log_prefix);
      g_fullscreen_desktop = 1;
    } else if(!strcmp("-noignbadacc", argv[i])) {
      glogf("%s Not ignoring bad memory accesses", parse_log_prefix);
      g_ignore_bad_acc = 0;
    } else if(!strcmp("-noignhalt", argv[i])) {
      glogf("%s Not ignoring code red halts", parse_log_prefix);
      g_ignore_halts = 0;
    } else if(!strcmp("-mem", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-mem' missing argument", parse_log_prefix);
        exit(1);
      }
      g_mem_size_exp = strtol(argv[i+1], 0, 0) & 0x00ff0000;
      glogf("%s Using %d as memory size", parse_log_prefix, g_mem_size_exp);
      i++;
    } else if(!strcmp("-skip", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-skip' missing argument", parse_log_prefix);
        exit(1);
      }
      skip_amt = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as skip_amt", parse_log_prefix, skip_amt);
      g_screen_redraw_skip_amt = skip_amt;
      i++;
    } else if(!strcmp("-audio", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-audio' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as audio enable val", parse_log_prefix, tmp1);
      g_audio_enable = tmp1;
      i++;
    } else if(!strcmp("-arate", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-arate' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as preferred audio rate", parse_log_prefix, tmp1);
      g_preferred_rate = tmp1;
      i++;
    } else if(!strcmp("-v", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-v' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Setting Verbose = 0x%03x", parse_log_prefix, tmp1);
      Verbose = tmp1;
      i++;
    } else if(!strcmp("-display", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-display' missing argument", parse_log_prefix);
        exit(1);
      }
      glogf("%s Using %s as display", parse_log_prefix, argv[i+1]);
      sprintf(g_display_env, "DISPLAY=%s", argv[i+1]);
      putenv(&g_display_env[0]);
      i++;
    } else if(!strcmp("-noshm", argv[i])) {
      glogf("%s Not using X shared memory", parse_log_prefix);
      g_use_shmem = 0;
    } else if(!strcmp("-joy", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-joy' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
      glogf("%s Setting joystick number %d", parse_log_prefix, tmp1);
      g_joystick_number = tmp1;
      i++;
    } else if(!strcmp("-joy-x", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-joy-x' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
      glogf("%s Setting joystick X axis %d", parse_log_prefix, tmp1);
      g_joystick_x_axis = tmp1;
      i++;
    } else if(!strcmp("-joy-y", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-joy-y' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
      glogf("%s Setting joystick Y axis %d", parse_log_prefix, tmp1);
      g_joystick_y_axis = tmp1;
      i++;
    } else if(!strcmp("-joy-x2", argv[i])) {
        if((i+1) >= argc) {
          glogf("%s Error, option '-joy-x2' missing argument", parse_log_prefix);
          exit(1);
        }
        tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
        glogf("%s Setting joystick X2 axis %d", parse_log_prefix, tmp1);
        g_joystick_x2_axis = tmp1;
        i++;
    } else if(!strcmp("-joy-y2", argv[i])) {
        if((i+1) >= argc) {
          glogf("%s Error, option '-joy-y2' missing argument", parse_log_prefix);
          exit(1);
        }
        tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
        glogf("%s Setting joystick Y2 axis %d", parse_log_prefix, tmp1);
        g_joystick_y2_axis = tmp1;
        i++;
    } else if(!strcmp("-joy-b0", argv[i])) {
        if((i+1) >= argc) {
          glogf("%s Error, option '-joy-b0' missing argument", parse_log_prefix);
          exit(1);
        }
        tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
        glogf("%s Setting joystick Button 0 to Gamepad %d", parse_log_prefix, tmp1);
        g_joystick_button_0 = tmp1;
        i++;
    } else if(!strcmp("-joy-b1", argv[i])) {
        if((i+1) >= argc) {
          glogf("%s Error, option '-joy-b1' missing argument", parse_log_prefix);
          exit(1);
        }
        tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
        glogf("%s Setting joystick Button 1 to Gamepad %d", parse_log_prefix, tmp1);
        g_joystick_button_1 = tmp1;
        i++;
    } else if(!strcmp("-joy-b2", argv[i])) {
        if((i+1) >= argc) {
          glogf("%s Error, option '-joy-b2' missing argument", parse_log_prefix);
          exit(1);
        }
        tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
        glogf("%s Setting joystick Button 2 to Gamepad %d", parse_log_prefix, tmp1);
        g_joystick_button_2 = tmp1;
        i++;
    } else if(!strcmp("-joy-b3", argv[i])) {
        if((i+1) >= argc) {
          glogf("%s Error, option '-joy-b3' missing argument", parse_log_prefix);
          exit(1);
        }
        tmp1 = strtol(argv[i+1], 0, 0); // no bounds check, not sure what ids we get
        glogf("%s Setting joystick Button 3 to Gamepad %d", parse_log_prefix, tmp1);
        g_joystick_button_3 = tmp1;
        i++;
    } else if(!strcmp("-dhr140", argv[i])) {
      glogf("%s Using simple dhires color map", parse_log_prefix);
      g_use_dhr140 = 1;
    } else if(!strcmp("-bw", argv[i])) {
      glogf("%s Forcing black-and-white hires modes", parse_log_prefix);
      g_cur_a2_stat |= ALL_STAT_COLOR_C021;
      g_use_bw_hires = 1;
    } else if(!strcmp("-scanline", argv[i])) {
      glogf("%s Enable scanline simulation", parse_log_prefix);
      if((i+1) >= argc) {
        glogf("%s Error, option '-scanline' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = parse_int(argv[i+1], 0, 100);
      glogf("%s Setting scanline simulator darkness to %d%%", parse_log_prefix, tmp1);
      g_scanline_simulator = tmp1;
      i++;
    } else if(!strcmp("-enet", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-enet' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as ethernet enable val", parse_log_prefix, tmp1);
      g_ethernet = tmp1;
      i++;
    } else if(!strcmp("-x", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-x' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as x val", parse_log_prefix, tmp1);
      g_startx = tmp1;
      i++;
    } else if(!strcmp("-y", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-y' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as y val", parse_log_prefix, tmp1);
      g_starty = tmp1;
      i++;
    } else if(!strcmp("-sw", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-sw' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as width val", parse_log_prefix, tmp1);
      g_startw = tmp1;
      i++;
    } else if(!strcmp("-sh", argv[i])) {
      if((i+1) >= argc) {
        glogf("%s Error, option '-sh' missing argument", parse_log_prefix);
        exit(1);
      }
      tmp1 = strtol(argv[i+1], 0, 0);
      glogf("%s Using %d as height val", parse_log_prefix, tmp1);
      g_starth = tmp1;
      i++;
    } else if(!strcmp("-config", argv[i])) {   // Config file passed
      if((i+1) >= argc) {
        glogf("%s Error, option '-config' missing argument", parse_log_prefix);
        exit(1);
      }
      glogf("%s Using %s as configuration file", parse_log_prefix, argv[i+1]);

      g_config_gsport_name_list[0] = argv[i+1]; // overwrite default list with passed item as sole option
      g_config_gsport_name_list[1] = 0; // terminate string array
      i++;
    } else if (!strcmp("-ssdir", argv[i])) {  // screenshot directory passed
      strcpy(g_config_gsport_screenshot_dir, argv[i+1]);
      struct stat path_stat;
      stat(g_config_gsport_screenshot_dir, &path_stat); // (weakly) validate path
      if (!S_ISDIR(path_stat.st_mode)) {
        strcpy(g_config_gsport_screenshot_dir, "./");
      }
      glogf("%s Using %s for screenshot path", parse_log_prefix, g_config_gsport_screenshot_dir);
      i++;
    } else if(!strcmp("-debugport", argv[i])) {     // Debug port passed
        if((i+1) >= argc) {
          glogf("%s Error, option '-debugport' missing argument", parse_log_prefix);
          exit(1);
        }
        g_dbg_enable_port = strtol(argv[i+1], 0, 0);
        glogf("%s Using %d for debug port", parse_log_prefix, g_dbg_enable_port);
        i++;
    } else {
      if ((i == (argc - 1)) && (strncmp("-", argv[i], 1) != 0)) {
        final_arg = argv[i];
      } else {
        glogf("%s Error, bad option: %s for debug port", parse_log_prefix, argv[i]);
        exit(3);
      }
    }
  }
}

void help_exit() {
  printf(" USAGE: \n\n");
  printf("   ./gsport                           # simple - uses default config.txt\n");
  printf("   ./gsport -config games_hds.gsp     # set custom config file\n\n");
  printf(" You need to supply your own Apple IIgs Firmware ROM image.\n");
  printf(" Press F4 when running gsport to enter config menu and select ROM image location.\n");
  printf(" Or copy the ROM image to the gsport directory.\n");
  printf(" It will search for:   \"ROM\", \"ROM.01\", \"ROM.03\" \n\n\n");
  printf("  Other command line options: \n\n");
  printf("    -badrd                Halt on bad reads\n");
  printf("    -noignbadacc          Don’t ignore bad memory accesses\n");
  printf("    -noignhalt            Don’t ignore code red halts\n");
  printf("    -test                 Allow testing\n");
  printf("    -joy                  Set joystick number\n");
  printf("    -bw                   Force B/W modes\n");
  printf("    -dhr140               Use simple double-hires color map\n");
  printf("    -fullscreen           Attempt to start emulator in fullscreen\n");
  printf("    -highdpi              Attempt to open window in high DPI\n");
  printf("    -borderless           Attempt to create borderless window\n");
  printf("    -resizeable           Allow you to resize window (non-integral scaling to pixel)\n");
  printf("    -mem value            Set memory size to value\n");
  printf("    -skip value           Set skip_amt to value\n");
  printf("    -audio value          Set audio enable to value\n");
  printf("    -arate value          Set preferred audio rate to value\n");
  printf("    -enet value           Set ethernet to value\n");
  printf("    -config value         Set config file to value\n");
  printf("    -debugport value      Set debugport to value\n");
  printf("    -ssdir value          Set screenshot save directory to value\n");
  printf("    -scanline value       Enable scanline simulator at value %%\n");
  printf("    -x value              Open emulator window at x value\n");
  printf("    -y value              Open emulator window at y value\n");
  printf("    -sw value             Scale window to sw pixels wide\n");
  printf("    -sh value             Scale window to sh pixels high\n");
  printf("    -novsync              Don't force emulator to sync each frame\n");
  printf("    -fulldesk             Use desktop 'fake' fullscreen mode\n");
  //printf("    -v value              Set verbose flags to value\n\n");
  printf("  Note: The final argument, if not a flag, will be tried as a mountable device.\n\n");
  exit(1);
}
