/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2013 by GSport contributors
 
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

/*
 * fbdriver - Linux fullscreen framebuffer graphics driver
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <termios.h>
#include <sys/mman.h>
#include "defc.h"

void quitEmulator();
extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];
extern word32 g_a2_screen_buffer_changed;
extern word32 g_c025_val;
#define SHIFT_DOWN	( (g_c025_val & 0x01) )
#define CTRL_DOWN	( (g_c025_val & 0x02) )
#define CAPS_LOCK_DOWN	( (g_c025_val & 0x04) )
#define OPTION_DOWN	( (g_c025_val & 0x40) )
#define CMD_DOWN	( (g_c025_val & 0x80) )
extern int g_video_act_margin_left;
extern int g_video_act_margin_right;
extern int g_video_act_margin_top;
extern int g_video_act_margin_bottom;
extern int g_lores_colors[];
extern int g_cur_a2_stat;
extern int g_a2vid_palette;
extern int g_installed_full_superhires_colormap;
extern int g_mouse_raw_x;
extern int g_mouse_raw_y;

int keycode_to_a2code[128] = 
{
    -1,   // KEY_RESERVED
    0x35, // KEY_ESC		
    0x12, // KEY_1
    0x13, // KEY_2
    0x14, // KEY_3
    0x15, // KEY_4
    0x17, // KEY_5
    0x16, // KEY_6
    0x1A, // KEY_7
    0x1C, // KEY_8
    0x19, // KEY_9
    0x1D, // KEY_0
    0x1B, // KEY_MINUS
    0x18, // KEY_EQUAL
    0x3B, // KEY_BACKSPACE0
    0x30, // KEY_TAB
    0x0C, // KEY_Q
    0x0D, // KEY_W
    0x0E, // KEY_E
    0x0F, // KEY_R
    0x11, // KEY_T
    0x10, // KEY_Y
    0x20, // KEY_U
    0x22, // KEY_I
    0x1F, // KEY_O
    0x23, // KEY_P
    0x21, // KEY_LEFTBRACE
    0x1E, // KEY_RIGHTBRACE
    0x24, // KEY_ENTER
    0x36, // KEY_LEFTCTRL
    0x00, // KEY_A
    0x01, // KEY_S
    0x02, // KEY_D
    0x03, // KEY_F
    0x05, // KEY_G
    0x04, // KEY_H
    0x26, // KEY_J
    0x28, // KEY_K
    0x25, // KEY_L
    0x29, // KEY_SEMICOLON
    0x27, // KEY_APOSTROPHE
    0x32, // KEY_GRAVE
    0x38, // KEY_LEFTSHIFT
    0x2A, // KEY_BACKSLASH
    0x06, // KEY_Z
    0x07, // KEY_X
    0x08, // KEY_C
    0x09, // KEY_V
    0x0B, // KEY_B
    0x2D, // KEY_N
    0x2E, // KEY_M
    0x2B, // KEY_COMMA
    0x2F, // KEY_DOT
    0x2C, // KEY_SLASH
    0x38, // KEY_RIGHTSHIFT
    0x43, // KEY_KPASTERISK
    0x37, // KEY_LEFTALT
    0x31, // KEY_SPACE
    0x39, // KEY_CAPSLOCK
    0x7A, // KEY_F1
    0x78, // KEY_F2
    0x63, // KEY_F3
    0x76, // KEY_F4
    0x60, // KEY_F5
    0x61, // KEY_F6
    0x62, // KEY_F7
    0x64, // KEY_F8
    0x65, // KEY_F9
    0x6D, // KEY_F10
    0x47, // KEY_NUMLOCK
    0x37, // KEY_SCROLLLOCK
    0x59, // KEY_KP7
    0x5B, // KEY_KP8
    0x5C, // KEY_KP9
    0x4E, // KEY_KPMINUS
    0x56, // KEY_KP4
    0x57, // KEY_KP5
    0x58, // KEY_KP6
    0x45, // KEY_KPPLUS
    0x53, // KEY_KP1
    0x54, // KEY_KP2
    0x55, // KEY_KP3
    0x52, // KEY_KP0
    0x41, // KEY_KPDOT
    -1,    
    -1,   // KEY_ZENKAKUHANKAKU
    -1,   // KEY_102ND
    0x67, // KEY_F11
    0x6F, // KEY_F12
    -1,   // KEY_RO
    -1,   // KEY_KATAKANA
    -1,   // KEY_HIRAGANA
    -1,   // KEY_HENKAN
    -1,   // KEY_KATAKANAHIRAGANA
    -1,   // KEY_MUHENKAN
    -1,   // KEY_KPJPCOMMA
    0x4C, // KEY_KPENTER
    0x36, // KEY_RIGHTCTRL
    0x4B, // KEY_KPSLASH
    0x7F, // KEY_SYSRQ
    0x37, // KEY_RIGHTALT
    0x6E, // KEY_LINEFEED
    0x73, // KEY_HOME
    0x3E, // KEY_UP
    0x74, // KEY_PAGEUP
    0x3B, // KEY_LEFT
    0x3C, // KEY_RIGHT
    0x77, // KEY_END
    0x3D, // KEY_DOWN
    0x79, // KEY_PAGEDOWN
    0x72, // KEY_INSERT
    0x33, // KEY_DELETE
    -1,   // KEY_MACRO
    -1,   // KEY_MUTE
    -1,   // KEY_VOLUMEDOWN
    -1,   // KEY_VOLUMEUP
    0x7F, // KEY_POWER	/* SC System Power Down */
    0x51, // KEY_KPEQUAL
    0x4E, // KEY_KPPLUSMINUS
    -1,   // KEY_PAUSE
    -1,   // KEY_SCALE	/* AL Compiz Scale (Expose) */    
    0x2B, // KEY_KPCOMMA
    -1,   // KEY_HANGEUL
    -1,   // KEY_HANJA
    -1,   // KEY_YEN
    0x3A, // KEY_LEFTMETA
    0x3A, // KEY_RIGHTMETA
    -1    // KEY_COMPOSE
};
struct termios org_tio;
struct fb_var_screeninfo orig_vinfo;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
int    g_use_shmem     = 1;
int    g_screen_mdepth = 8;
#define	MOUSE_LBTN_DOWN		0x01
#define	MOUSE_MBTN_DOWN		0x02
#define	MOUSE_RBTN_DOWN		0x04
#define	MOUSE_LBTN_UP		0x00
#define	MOUSE_MBTN_UP		0x00
#define	MOUSE_RBTN_UP		0x00
#define	MOUSE_BTN_ACTIVE	0x07
#define UPDATE_INPUT_MOUSE	0x10
#define MAX_EVDEV               8
char *fb_ptr, g_inputstate = 0;
int  evfd[MAX_EVDEV], evdevs, termfd, fbfd = 0;
/*
 * Clean up
 */
void xdriver_end(void)
{
    char c;
    static char xexit = 0;
    if (!xexit)
    {
	// cleanup
	munmap(fb_ptr, finfo.smem_len);
	ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo);
	close(fbfd);
	if (termfd > 0)
	{
	    // Flush input
	    while (read(termfd, &c, 1) == 1);
	    ioctl(termfd, KDSETMODE, KD_TEXT);
	    tcsetattr(termfd, TCSANOW, &org_tio);
	    close(termfd);
	}
	fclose(stdout);
	fclose(stderr);
	while (evdevs--)
	    close(evfd[evdevs]);
	xexit = 1;
    }
}
/*
 * Init framebuffer and input
 */
void dev_video_init(void)
{
    int i;
    char evdevname[20];
    struct termios termio;

    // Set graphics mode on console
    if ((termfd = open("/dev/tty", O_RDWR)) < 0)
    {
	fprintf(stderr, "Error opening tty device.\n");
	exit(-1);
    }
    // Save input settings.
    tcgetattr(termfd, &termio); /* save current port settings */
    memcpy(&org_tio, &termio, sizeof(struct termios));
    ioctl(termfd, KDSETMODE, KD_GRAPHICS);
    // Open the file for reading and writing
    if ((fbfd = open("/dev/fb0", O_RDWR)) < 0)
    {
	fprintf(stderr, "Error opening framebuffer device.\n");
	ioctl(termfd, KDSETMODE, KD_TEXT);
	exit(-1);
    }
    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
    {
	fprintf(stderr, "Error reading variable screen information.\n");
	ioctl(termfd, KDSETMODE, KD_TEXT);
	exit(-1);
    }
    // Store for reset(copy vinfo to vinfo_orig)
    memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));
    // Change variable info
    vinfo.bits_per_pixel = 8;
    // Change resolution
    vinfo.xres = 640;
    vinfo.yres = 400;
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo))
    {
	fprintf(stderr, "Error setting variable screen information (640x400x8).\n");	
	ioctl(termfd, KDSETMODE, KD_TEXT);
	exit(-1);
    }
    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
    {
	fprintf(stderr, "Error reading fixed screen information.\n");
	ioctl(termfd, KDSETMODE, KD_TEXT);
	exit(-1);
    }
    // map fb to user mem
    fb_ptr = (char*)mmap(0,
			 finfo.smem_len,
			 PROT_READ|PROT_WRITE,
			 MAP_SHARED,
			 fbfd,
			 0);
    if ((int)fb_ptr == -1)
    {
	printf("Failed to mmap framebuffer.\n");
	ioctl(termfd, KDSETMODE, KD_TEXT);
	exit (-1);
    }
    video_get_kimages();
    for (i = 0; i < 256; i++)
    {
	video_update_color_raw(i, g_lores_colors[i & 0xf]);
	g_a2palette_8to1624[i] = g_palette_8to1624[i];
    }
    fclose(stdin);
    freopen("gsport.log", "w+", stdout);
    freopen("gsport.err", "w+", stderr);
    termio.c_cflag     = /*BAUDRATE | CRTSCTS |*/ CS8 | CLOCAL | CREAD;
    termio.c_iflag     = IGNPAR;
    termio.c_oflag     = 0;
    termio.c_lflag     = 0; /* set input mode (non-canonical, no echo,...) */
    termio.c_cc[VTIME] = 0; /* inter-character timer unused */
    termio.c_cc[VMIN]  = 0; /* non-blocking read */
    tcsetattr(termfd, TCSANOW, &termio);
    // Open input event devices
    for (evdevs = 0; evdevs < MAX_EVDEV; evdevs++)
    {
        sprintf(evdevname, "/dev/input/event%c", evdevs + '0');
        if ((evfd[evdevs] = open(evdevname, O_RDONLY|O_NONBLOCK)) < 0)
	    break;
    }
    g_video_act_margin_left   = 0;
    g_video_act_margin_right  = 1;
    g_video_act_margin_top    = 0;
    g_video_act_margin_bottom = 0;
}
/*
 * Colormap
 */
__u16 cmapred[256], cmapgreen[256], cmapblue[256];
int cmapstart, cmaplen, cmapdirty = 0;
void x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
    cmapred[col_num]   = red   | (red   << 8);
    cmapgreen[col_num] = green | (green << 8);
    cmapblue[col_num]  = blue  | (blue  << 8);
    if (cmapdirty == 0)
    {
	cmapstart = col_num;
	cmaplen   = 1;
	cmapdirty = 1;
    }
    else
    {
	if (col_num < cmapstart)
	{
	    cmaplen += cmapstart - col_num;
	    cmapstart = col_num;
	}
	else if (col_num > cmapstart + cmaplen)
	    cmaplen = col_num - cmapstart + 1;
    }
}
void x_update_physical_colormap(void)
{
    struct fb_cmap fbcol;
    if (cmapdirty)
    {
	cmapdirty       = 0;
	fbcol.start     = cmapstart;
	fbcol.len       = cmaplen;
	fbcol.red       = cmapred;
	fbcol.green     = cmapgreen;
	fbcol.blue      = cmapblue;
	fbcol.transp    = NULL;
	ioctl(fbfd, FBIOPUTCMAP, &fbcol);
    }
}
void show_xcolor_array(void)
{
}
/*
 * Screen update
 */
void x_get_kimage(Kimage *kimage_ptr)
{
    kimage_ptr->data_ptr = (byte *)malloc(kimage_ptr->width_req * kimage_ptr->height);
}
void x_release_kimage(Kimage* kimage_ptr)
{
    if (kimage_ptr->data_ptr)
	if (kimage_ptr->width_req != 640 || kimage_ptr->height != 400)
	    free(kimage_ptr->data_ptr);
    kimage_ptr->data_ptr = NULL;
}
void x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy, int width, int height)
{
    byte *src_ptr, *dst_ptr;

    // Copy sub-image to framebuffer
    dst_ptr = (byte *)fb_ptr       + desty * finfo.line_length    + destx;
    src_ptr = kimage_ptr->data_ptr + srcy * kimage_ptr->width_act + srcx;
    while (height--)
    {
	memcpy(dst_ptr, src_ptr, width);
	dst_ptr += finfo.line_length;
	src_ptr += kimage_ptr->width_act;
    }
}
void x_push_done(void)
{
}
/*
 * NOP routines
 */
void x_dialog_create_gsport_conf(const char *str)
{
    // Just write the config file already...
    config_write_config_gsport_file();
}
int x_show_alert(int is_fatal, const char *str)
{
    // Not implemented yet
    adb_all_keys_up();
    clear_fatal_logs();
    return 0;
}
void x_redraw_status_lines(void)
{
}
void x_hide_pointer(int do_hide)
{
}
void x_auto_repeat_on(int must)
{
}
void x_full_screen(int do_full)
{
}
int x_calc_ratio(float x, float y)
{
    return 1;
}
/*
 * Input handling
 */
void check_input_events(void)
{
    struct input_event ev;
    int i;
    
    for (i = 0; i < evdevs; i++)
	// Check input events
	while (read(evfd[i], &ev, sizeof(struct input_event)) == sizeof(struct input_event))
	{
	    if (ev.type == EV_REL)
	    {
		if (ev.code == REL_X)
		{
		    g_mouse_raw_x += ev.value;
		    if (g_mouse_raw_x < 0)
			g_mouse_raw_x = 0;
		    if (g_mouse_raw_x > 639)
			g_mouse_raw_x = 639;
		}
		else // REL_Y
		{
		    g_mouse_raw_y += ev.value;
		    if (g_mouse_raw_y < 0)
			g_mouse_raw_y = 0;
		    if (g_mouse_raw_y > 399)
			g_mouse_raw_y = 399;
		}
		g_inputstate |= UPDATE_INPUT_MOUSE;
	    }
	    else if (ev.type == EV_KEY)
	    {
		if (ev.code < 128)
		{
		    if ((ev.code == KEY_F10) && SHIFT_DOWN)
		    {
			//quitEmulator();
			xdriver_end();
			exit(0);
		    }
		    if (keycode_to_a2code[ev.code] >= 0)
			adb_physical_key_update(keycode_to_a2code[ev.code], !ev.value);
		}
		else if (ev.code == BTN_LEFT)
		{
		    g_inputstate = ev.value ? UPDATE_INPUT_MOUSE | MOUSE_LBTN_DOWN
			                    : UPDATE_INPUT_MOUSE | MOUSE_LBTN_UP;
		}
	    }
	    else if (ev.type == EV_SYN)
	    {
		if (g_inputstate & UPDATE_INPUT_MOUSE)
		    update_mouse(g_mouse_raw_x, g_mouse_raw_y, g_inputstate & MOUSE_BTN_ACTIVE, MOUSE_BTN_ACTIVE);
		g_inputstate &= ~UPDATE_INPUT_MOUSE;
	    }
	}
}
static void sig_bye(int signo)
{
    xdriver_end();
    exit (-1);
}
/*
 * Application entrypoint
 */
int main(int argc,char *argv[])
{
    if (signal(SIGINT, sig_bye) == SIG_ERR)
	exit(-1);
    if (signal(SIGHUP, sig_bye) == SIG_ERR)
	exit(-1);
    gsportmain(argc, argv);
    xdriver_end();
    return 0;
}
