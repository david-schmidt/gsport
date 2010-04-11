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

// $KmKId: protos_windriver.h,v 1.4 2004-03-23 17:27:26-05 kentd Exp $

/* END_HDR */

/* windriver.c */
int win_update_mouse(int x, int y, int button_states, int buttons_valid);
void win_event_mouse(WPARAM wParam, LPARAM lParam);
void win_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags);
void win_event_quit(HWND hwnd);
void win_event_redraw(void);
LRESULT CALLBACK win_event_handler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
int main(int argc, char **argv);
void check_input_events(void);
void x_update_color(int col_num, int red, int green, int blue, word32 rgb);
void x_update_physical_colormap(void);
void show_xcolor_array(void);
void xdriver_end(void);
void x_get_kimage(Kimage *kimage_ptr);
void dev_video_init(void);
void x_redraw_status_lines(void);
void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height);
void x_push_done(void);
void x_auto_repeat_on(int must);
void x_auto_repeat_off(int must);
void x_hide_pointer(int do_hide);

