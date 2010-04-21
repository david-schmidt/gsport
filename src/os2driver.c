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
#include "protos.h"

extern int Verbose;

extern int g_warp_pointer;
extern int g_screen_depth;
extern int g_force_depth;
int g_screen_mdepth = 0;

extern int g_quit_sim_now;

int	g_use_shmem = 1;
int	g_has_focus = 0;
int	g_auto_repeat_on = -1;

extern Kimage g_mainwin_kimage;

int	g_main_height = 0;

int	g_win_capslock_down = 0;

extern int g_border_sides_refresh_needed;
extern int g_border_special_refresh_needed;
extern int g_status_refresh_needed;

extern int g_lores_colors[];
extern int g_cur_a2_stat;

extern int g_a2vid_palette;

extern int g_installed_full_superhires_colormap;

extern int g_screen_redraw_skip_amt;

extern word32 g_a2_screen_buffer_changed;


int
win_nonblock_read_stdin(int fd, char *bufptr, int len)
{
}

void
x_dialog_create_gsport_conf(const char *str)
{
}

int
x_show_alert(int is_fatal, const char *str)
{
	return 0;
}



int
main(int argc, char **argv)
{
	printf("In OS/2 main...\n");

	// Call gsportmain
	return gsportmain(argc, argv);
}


void
check_input_events()
{
}


void
x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
}

void
x_update_physical_colormap()
{
}

void
show_xcolor_array()
{
}


void
xdriver_end()
{
	printf("OS/2 driver_end\n");
}


void
x_get_kimage(Kimage *kimage_ptr)
{
	return;
}


void
dev_video_init()
{
	printf("Preparing graphics system\n");
}

void
x_redraw_status_lines()
{
}


void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
	int width, int height)
{
}

void
x_push_done()
{
}

void
x_auto_repeat_on(int must)
{
}

void
x_auto_repeat_off(int must)
{
}

void
x_hide_pointer(int do_hide)
{
}

void
x_full_screen(int do_full)
{
	return;
}
