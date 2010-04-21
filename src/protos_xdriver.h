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

const char rcsid_protos_x_h[] = "@(#)$KmKId: protos_xdriver.h,v 1.19 2004-11-15 16:24:13-05 kentd Exp $";

/* END_HDR */

/* xdriver.c */
int main(int argc, char **argv);
void x_dialog_create_gsport_conf(const char *str);
int x_show_alert(int is_fatal, const char *str);
void x_update_color(int col_num, int red, int green, int blue, word32 rgb);
void x_update_physical_colormap(void);
void show_xcolor_array(void);
int my_error_handler(Display *display, XErrorEvent *ev);
void xdriver_end(void);
void show_colormap(char *str, Colormap cmap, int index1, int index2, int index3);
void x_badpipe(int signum);
void dev_video_init(void);
Visual *x_try_find_visual(int depth, int screen_num, XVisualInfo **visual_list_ptr);
void x_set_mask_and_shift(word32 x_mask, word32 *mask_ptr, int *shift_left_ptr, int *shift_right_ptr);
int xhandle_shm_error(Display *display, XErrorEvent *event);
void x_get_kimage(Kimage *kimage_ptr);
int get_shm(Kimage *kimage_ptr);
void get_ximage(Kimage *kimage_ptr);
void x_redraw_status_lines(void);
void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height);
void x_push_done(void);
int x_update_mouse(int raw_x, int raw_y, int button_states, int buttons_valid);
void check_input_events(void);
void x_hide_pointer(int do_hide);
void handle_keysym(XEvent *xev_in);
int x_keysym_to_a2code(int keysym, int is_up);
void x_update_modifier_state(int state);
void x_auto_repeat_on(int must);
void x_auto_repeat_off(int must);
void x_full_screen(int do_full);

