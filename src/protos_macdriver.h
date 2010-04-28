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

/* END_HDR */

/* macdriver.c */
pascal OSStatus quit_event_handler(EventHandlerCallRef call_ref, EventRef event, void *ignore);
void show_simple_alert(char *str1, char *str2, char *str3, int num);
void x_dialog_create_gsport_conf(const char *str);
int x_show_alert(int is_fatal, const char *str);
pascal OSStatus my_cmd_handler(EventHandlerCallRef handlerRef, EventRef event, void *userdata);
void update_window(void);
void show_event(UInt32 event_class, UInt32 event_kind, int handled);
pascal OSStatus my_win_handler(EventHandlerCallRef handlerRef, EventRef event, void *userdata);
pascal OSStatus dummy_event_handler(EventHandlerCallRef call_ref, EventRef in_event, void *ignore);
void mac_update_modifiers(word32 state);
void mac_warp_mouse(void);
void check_input_events(void);
void temp_run_application_event_loop(void);
int main(int argc, char *argv[]);
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
void x_full_screen(int do_full);
void update_main_window_size(void);
