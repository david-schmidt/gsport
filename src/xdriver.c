/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2012 by GSport contributors
 
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

# if !defined(__CYGWIN__) && !defined(__POWERPC__)
/* No shared memory on Cygwin */
# define X_SHARED_MEM
#endif /* CYGWIN */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#ifdef X_SHARED_MEM
# include <sys/ipc.h>
# include <sys/shm.h>
# include <X11/extensions/XShm.h>
#endif

int XShmQueryExtension(Display *display);

#include "defc.h"
#include "protos_xdriver.h"

#define FONT_NAME_STATUS	"8x13"

extern int Verbose;

extern int g_warp_pointer;
extern int g_screen_depth;
extern int g_force_depth;
int g_screen_mdepth = 0;

extern int _Xdebug;

extern int g_send_sound_to_file;

extern int g_quit_sim_now;

int	g_has_focus = 0;
int	g_auto_repeat_on = -1;
int	g_x_shift_control_state = 0;


Display *g_display = 0;
Visual	*g_vis = 0;
Window g_a2_win;
GC g_a2_winGC;
XFontStruct *g_text_FontSt;
Colormap g_a2_colormap = 0;
Colormap g_default_colormap = 0;
int	g_needs_cmap = 0;

extern word32 g_red_mask;
extern word32 g_green_mask;
extern word32 g_blue_mask;
extern int g_red_left_shift;
extern int g_green_left_shift;
extern int g_blue_left_shift;
extern int g_red_right_shift;
extern int g_green_right_shift;
extern int g_blue_right_shift;

#ifdef X_SHARED_MEM
int g_use_shmem = 1;
#else
int g_use_shmem = 0;
#endif

extern Kimage g_mainwin_kimage;

extern int Max_color_size;

XColor g_xcolor_a2vid_array[256];

extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

int	g_alt_left_up = 1;
int	g_alt_right_up = 1;

extern word32 g_full_refresh_needed;

extern int g_border_sides_refresh_needed;
extern int g_border_special_refresh_needed;
extern int g_status_refresh_needed;

extern int g_lores_colors[];
extern int g_cur_a2_stat;

extern int g_a2vid_palette;

extern int g_installed_full_superhires_colormap;

extern int g_screen_redraw_skip_amt;

extern word32 g_a2_screen_buffer_changed;

extern char *g_status_ptrs[MAX_STATUS_LINES];

Cursor	g_cursor;
Pixmap	g_cursor_shape;
Pixmap	g_cursor_mask;

XColor	g_xcolor_black = { 0, 0x0000, 0x0000, 0x0000, DoRed|DoGreen|DoBlue, 0 };
XColor	g_xcolor_white = { 0, 0xffff, 0xffff, 0xffff, DoRed|DoGreen|DoBlue, 0 };

int g_depth_attempt_list[] = { 16, 24, 15, 8 };


#define X_EVENT_LIST_ALL_WIN						\
	(ExposureMask | ButtonPressMask | ButtonReleaseMask |		\
	 OwnerGrabButtonMask | KeyPressMask | KeyReleaseMask |		\
	 KeymapStateMask | ColormapChangeMask | FocusChangeMask)

#define X_BASE_WIN_EVENT_LIST						\
	(X_EVENT_LIST_ALL_WIN | PointerMotionMask | ButtonMotionMask)

#define X_A2_WIN_EVENT_LIST						\
	(X_BASE_WIN_EVENT_LIST)

int	g_num_a2_keycodes = 0;

int a2_key_to_xsym[][3] = {
	{ 0x35,	XK_Escape,	0 },
	{ 0x7a,	XK_F1,	0 },
	{ 0x78,	XK_F2,	0 },
	{ 0x63,	XK_F3,	0 },
	{ 0x76,	XK_F4,	0 },
	{ 0x60,	XK_F5,	0 },
	{ 0x61,	XK_F6,	0 },
	{ 0x62,	XK_F7,	0 },
	{ 0x64,	XK_F8,	0 },
	{ 0x65,	XK_F9,	0 },
	{ 0x6d,	XK_F10,	0 },
	{ 0x67,	XK_F11,	0 },
	{ 0x6f,	XK_F12,	0 },
	{ 0x69,	XK_F13,	0 },
	{ 0x6b,	XK_F14,	0 },
	{ 0x71,	XK_F15,	0 },
	{ 0x7f, XK_Pause, XK_Break },
	{ 0x32,	'`', '~' },		/* Key number 18? */
	{ 0x12,	'1', '!' },
	{ 0x13,	'2', '@' },
	{ 0x14,	'3', '#' },
	{ 0x15,	'4', '$' },
	{ 0x17,	'5', '%' },
	{ 0x16,	'6', '^' },
	{ 0x1a,	'7', '&' },
	{ 0x1c,	'8', '*' },
	{ 0x19,	'9', '(' },
	{ 0x1d,	'0', ')' },
	{ 0x1b,	'-', '_' },
	{ 0x18,	'=', '+' },
	{ 0x33,	XK_BackSpace, 0 },
	{ 0x72,	XK_Insert, XK_Help },	/* Help? */
/*	{ 0x73,	XK_Home, 0 },		alias XK_Home to be XK_KP_Equal! */
	{ 0x74,	XK_Page_Up, 0 },
	{ 0x47,	XK_Num_Lock, XK_Clear },	/* Clear */
	{ 0x51,	XK_KP_Equal, XK_Home },		/* Note XK_Home alias! */
	{ 0x4b,	XK_KP_Divide, 0 },
	{ 0x43,	XK_KP_Multiply, 0 },

	{ 0x30,	XK_Tab, 0 },
	{ 0x0c,	'q', 'Q' },
	{ 0x0d,	'w', 'W' },
	{ 0x0e,	'e', 'E' },
	{ 0x0f,	'r', 'R' },
	{ 0x11,	't', 'T' },
	{ 0x10,	'y', 'Y' },
	{ 0x20,	'u', 'U' },
	{ 0x22,	'i', 'I' },
	{ 0x1f,	'o', 'O' },
	{ 0x23,	'p', 'P' },
	{ 0x21,	'[', '{' },
	{ 0x1e,	']', '}' },
	{ 0x2a,	0x5c, '|' },	/* backslash, bar */
	{ 0x75,	XK_Delete, 0 },
	{ 0x77,	XK_End, 0 },
	{ 0x79,	XK_Page_Down, 0 },
	{ 0x59,	XK_KP_7, XK_KP_Home },
	{ 0x5b,	XK_KP_8, XK_KP_Up },
	{ 0x5c,	XK_KP_9, XK_KP_Page_Up },
	{ 0x4e,	XK_KP_Subtract, 0 },

	{ 0x39,	XK_Caps_Lock, 0 },
	{ 0x00,	'a', 'A' },
	{ 0x01,	's', 'S' },
	{ 0x02,	'd', 'D' },
	{ 0x03,	'f', 'F' },
	{ 0x05,	'g', 'G' },
	{ 0x04,	'h', 'H' },
	{ 0x26,	'j', 'J' },
	{ 0x28,	'k', 'K' },
	{ 0x25,	'l', 'L' },
	{ 0x29,	';', ':' },
	{ 0x27,	0x27, '"' },	/* single quote */
	{ 0x24,	XK_Return, 0 },
	{ 0x56,	XK_KP_4, XK_KP_Left },
	{ 0x57,	XK_KP_5, 0 },
	{ 0x58,	XK_KP_6, XK_KP_Right },
	{ 0x45,	XK_KP_Add, 0 },

	{ 0x38,	XK_Shift_L, XK_Shift_R },
	{ 0x06,	'z', 'Z' },
	{ 0x07,	'x', 'X' },
	{ 0x08,	'c', 'C' },
	{ 0x09,	'v', 'V' },
	{ 0x0b,	'b', 'B' },
	{ 0x2d,	'n', 'N' },
	{ 0x2e,	'm', 'M' },
	{ 0x2b,	',', '<' },
	{ 0x2f,	'.', '>' },
	{ 0x2c,	'/', '?' },
	{ 0x3e,	XK_Up, 0 },
	{ 0x53,	XK_KP_1, XK_KP_End },
	{ 0x54,	XK_KP_2, XK_KP_Down },
	{ 0x55,	XK_KP_3, XK_KP_Page_Down },

	{ 0x36,	XK_Control_L, XK_Control_R },
	{ 0x3a,	XK_Print, XK_Sys_Req },		/* Option */
	{ 0x37,	XK_Scroll_Lock, 0 },		/* Command */
	{ 0x31,	' ', 0 },
	{ 0x3b,	XK_Left, 0 },
	{ 0x3d,	XK_Down, 0 },
	{ 0x3c,	XK_Right, 0 },
	{ 0x52,	XK_KP_0, XK_KP_Insert },
	{ 0x41,	XK_KP_Decimal, XK_KP_Separator },
	{ 0x4c,	XK_KP_Enter, 0 },
	{ -1, -1, -1 }
};

int
main(int argc, char **argv)
{
	return gsportmain(argc, argv);
}

void
x_dialog_create_gsport_conf(const char *str)
{
	// Just write the config file already...
	config_write_config_gsport_file();
}

int
x_show_alert(int is_fatal, const char *str)
{
	/* Not implemented yet */
	adb_all_keys_up();

	clear_fatal_logs();
	return 0;
}


#define MAKE_2(val)	( (val << 8) + val)

void
x_update_color(int col_num, int red, int green, int blue, word32 rgb)
{
	XColor *xcol;

	xcol = &(g_xcolor_a2vid_array[col_num]);
	xcol->red = MAKE_2(red);
	xcol->green = MAKE_2(green);
	xcol->blue = MAKE_2(blue);
	xcol->flags = DoRed | DoGreen | DoBlue;
}

void
x_update_physical_colormap()
{
	if(g_needs_cmap) {
		XStoreColors(g_display, g_a2_colormap,
				&g_xcolor_a2vid_array[0], Max_color_size);
	}
}

void
show_xcolor_array()
{
	int i;

	for(i = 0; i < 256; i++) {
		printf("%02x: %08x\n", i, g_palette_8to1624[i]);
			
#if 0
		printf("%02x: %04x %04x %04x, %02x %x\n",
			i, xcolor_array[i].red, xcolor_array[i].green,
			xcolor_array[i].blue, (word32)xcolor_array[i].pixel,
			xcolor_array[i].flags);
#endif
	}
}


int
my_error_handler(Display *display, XErrorEvent *ev)
{
	char msg[1024];
	XGetErrorText(display, ev->error_code, msg, 1000);
	printf("X Error code %s\n", msg);
	fflush(stdout);

	return 0;
}

void
xdriver_end()
{

	printf("xdriver_end\n");
	if(g_display) {
		x_auto_repeat_on(1);
		XFlush(g_display);
	}
}

void
show_colormap(char *str, Colormap cmap, int index1, int index2, int index3)
{
	XColor	xcol;
	int	i;
	int	pix;

	printf("Show colormap: %08x = %s, cmap cells: %d,%d,%d\n",
			(int)cmap, str, index1, index2, index3);
	for(i = 0; i < index1 + index2 + index3; i++) {
		pix = i;
		if(i >= index1) {
			pix = (i-index1)*index1;
			if(i >= (index1 + index2)) {
				pix = (i - index1 - index2)*index2*index1;
			}
		}
		if(i == 0 && index1 < 250) {
			pix = 0x842;
		}
		xcol.pixel = pix;
		XQueryColor(g_display, cmap, &xcol);
		printf("Cell %03x: pix: %03x, R:%04x, G:%04x, B:%04x\n",
			i, (int)xcol.pixel, xcol.red, xcol.green, xcol.blue);
	}
}

void
x_badpipe(int signum)
{
	/* restore normal sigpipe handling */
	signal(SIGPIPE, SIG_DFL);

	/* attempt to xset r */
	system("xset r");
	my_exit(5);
}

void
dev_video_init()
{
	int	tmp_array[0x80];
	XGCValues new_gc;
	XSetWindowAttributes win_attr;
	XSizeHints my_winSizeHints;
	XClassHint my_winClassHint;
	XTextProperty my_winText;
	XVisualInfo *visualList;
	char	**font_ptr;
	char	cursor_data;
	word32	create_win_list;
	int	depth;
	int	len;
	int	cmap_alloc_amt;
	int	cnt;
	int	font_height;
	int	base_height;
	int	screen_num;
	char	*myTextString[1];
	word32	lores_col;
	int	ret;
	int	i;
	int	keycode;

	printf("Preparing X Windows graphics system\n");
	ret = 0;

	signal(SIGPIPE, x_badpipe);

	g_num_a2_keycodes = 0;
	for(i = 0; i <= 0x7f; i++) {
		tmp_array[i] = 0;
	}
	for(i = 0; i < 0x7f; i++) {
		keycode = a2_key_to_xsym[i][0];
		if(keycode < 0) {
			g_num_a2_keycodes = i;
			break;
		} else if(keycode > 0x7f) {
			printf("a2_key_to_xsym[%d] = %02x!\n", i, keycode);
				exit(2);
		} else {
			if(tmp_array[keycode]) {
				printf("a2_key_to_x[%d] = %02x used by %d\n",
					i, keycode, tmp_array[keycode] - 1);
			}
			tmp_array[keycode] = i + 1;
		}
	}

#if 0
	printf("Setting _Xdebug = 1, makes X synchronous\n");
	_Xdebug = 1;
#endif

	g_display = XOpenDisplay(NULL);
	if(g_display == NULL) {
		fprintf(stderr, "Can't open display\n");
		exit(1);
	}

	vid_printf("Just opened display = %p\n", g_display);
	fflush(stdout);

	screen_num = DefaultScreen(g_display);

	len = sizeof(g_depth_attempt_list)/sizeof(int);
	if(g_force_depth > 0) {
		/* Only use the requested user depth */
		len = 1;
		g_depth_attempt_list[0] = g_force_depth;
	}
	g_vis = 0;
	for(i = 0; i < len; i++) {
		depth = g_depth_attempt_list[i];

		g_vis = x_try_find_visual(depth, screen_num,
			&visualList);
		if(g_vis != 0) {
			break;
		}
	}
	if(g_vis == 0) {
		fprintf(stderr, "Couldn't find any visuals at any depth!\n");
		exit(2);
	}

	g_default_colormap = XDefaultColormap(g_display, screen_num);
	if(!g_default_colormap) {
		printf("g_default_colormap == 0!\n");
		exit(4);
	}

	g_a2_colormap = -1;
	cmap_alloc_amt = AllocNone;
	if(g_needs_cmap) {
		cmap_alloc_amt = AllocAll;
	}
	g_a2_colormap = XCreateColormap(g_display,
			RootWindow(g_display,screen_num), g_vis,
					cmap_alloc_amt);

	vid_printf("g_a2_colormap: %08x, main: %08x\n",
			(word32)g_a2_colormap, (word32)g_default_colormap);

	if(g_needs_cmap && g_a2_colormap == g_default_colormap) {
		printf("A2_colormap = default colormap!\n");
		exit(4);
	}

	/* and define cursor */
	cursor_data = 0;
	g_cursor_shape = XCreatePixmapFromBitmapData(g_display,
		RootWindow(g_display,screen_num), &cursor_data, 1, 1, 1, 0, 1);
	g_cursor_mask = XCreatePixmapFromBitmapData(g_display,
		RootWindow(g_display,screen_num), &cursor_data, 1, 1, 1, 0, 1);

	g_cursor = XCreatePixmapCursor(g_display, g_cursor_shape,
			g_cursor_mask, &g_xcolor_black, &g_xcolor_white, 0, 0);

	XFreePixmap(g_display, g_cursor_shape);
	XFreePixmap(g_display, g_cursor_mask);

	XFlush(g_display);

	win_attr.event_mask = X_A2_WIN_EVENT_LIST;
	win_attr.colormap = g_a2_colormap;
	win_attr.backing_store = WhenMapped;
	win_attr.border_pixel = 1;
	win_attr.background_pixel = 0;
	if(g_warp_pointer) {
		win_attr.cursor = g_cursor;
	} else {
		win_attr.cursor = None;
	}

	vid_printf("About to a2_win, depth: %d\n", g_screen_depth);
	fflush(stdout);

	create_win_list = CWEventMask | CWBackingStore | CWCursor;
	create_win_list |= CWColormap | CWBorderPixel | CWBackPixel;

	base_height = X_A2_WINDOW_HEIGHT + (MAX_STATUS_LINES * 13);

	g_a2_win = XCreateWindow(g_display, RootWindow(g_display, screen_num),
		0, 0, BASE_WINDOW_WIDTH, base_height,
		0, g_screen_depth, InputOutput, g_vis,
		create_win_list, &win_attr);

	XSetWindowColormap(g_display, g_a2_win, g_a2_colormap);

	XFlush(g_display);

/* Check for XShm */
#ifdef X_SHARED_MEM
	if(g_use_shmem) {
		ret = XShmQueryExtension(g_display);
		if(ret == 0) {
			printf("XShmQueryExt ret: %d\n", ret);
			printf("not using shared memory\n");
			g_use_shmem = 0;
		} else {
			printf("Will use shared memory for X\n");
		}
	}
#endif

	video_get_kimages();
	if(g_screen_depth != 8) {
		video_get_kimage(&g_mainwin_kimage, 0, g_screen_depth,
				g_screen_mdepth);
	}

	if(!g_use_shmem) {
		if(g_screen_redraw_skip_amt < 0) {
			g_screen_redraw_skip_amt = 3;
		}
		printf("Not using shared memory, setting skip_amt = %d\n",
			g_screen_redraw_skip_amt);
	}

	/* Done with visualList now */
	XFree(visualList);

	for(i = 0; i < 256; i++) {
		g_xcolor_a2vid_array[i].pixel = i;
		lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}

	x_update_physical_colormap();

	g_installed_full_superhires_colormap = !g_needs_cmap;
	
	myTextString[0] = "GSport";

	XStringListToTextProperty(myTextString, 1, &my_winText);

	my_winSizeHints.flags = PSize | PMinSize | PMaxSize;
	my_winSizeHints.width = BASE_WINDOW_WIDTH;
	my_winSizeHints.height = base_height;
	my_winSizeHints.min_width = BASE_WINDOW_WIDTH;
	my_winSizeHints.min_height = base_height;
	my_winSizeHints.max_width = BASE_WINDOW_WIDTH;
	my_winSizeHints.max_height = base_height;
	my_winClassHint.res_name = "GSport";
	my_winClassHint.res_class = "GSport";

	XSetWMProperties(g_display, g_a2_win, &my_winText, &my_winText, 0,
		0, &my_winSizeHints, 0, &my_winClassHint);
	XMapRaised(g_display, g_a2_win);

	XSync(g_display, False);

	g_a2_winGC = XCreateGC(g_display, g_a2_win, 0, (XGCValues *) 0);
	font_ptr = XListFonts(g_display, FONT_NAME_STATUS, 4, &cnt);

	vid_printf("act_cnt of fonts: %d\n", cnt);
	for(i = 0; i < cnt; i++) {
		vid_printf("Font %d: %s\n", i, font_ptr[i]);
	}
	fflush(stdout);
	g_text_FontSt = XLoadQueryFont(g_display, FONT_NAME_STATUS);
	vid_printf("font # returned: %08x\n", (word32)(g_text_FontSt->fid));
	font_height = g_text_FontSt->ascent + g_text_FontSt->descent;
	vid_printf("font_height: %d\n", font_height);

	vid_printf("widest width: %d\n", g_text_FontSt->max_bounds.width);

	new_gc.font = g_text_FontSt->fid;
	new_gc.fill_style = FillSolid;
	XChangeGC(g_display, g_a2_winGC, GCFillStyle | GCFont, &new_gc);

	/* XSync(g_display, False); */
#if 0
/* MkLinux for Powermac depth 15 has bugs--this was to try to debug them */
	if(g_screen_depth == 15) {
		/* draw phony screen */
		ptr16 = (word16 *)dint_main_win;
		for(i = 0; i < 320*400; i++) {
			ptr16[i] = 0;
		}
		for(i = 0; i < 400; i++) {
			for(j = 0; j < 640; j++) {
				sh = (j / 20) & 0x1f;
				val = sh;
				val = val;
				*ptr16++ = val;
			}
		}
		XPutImage(g_display, g_a2_win, g_a2_winGC, xint_main_win,
			0, 0,
			BASE_MARGIN_LEFT, BASE_MARGIN_TOP,
			640, 400);
		XFlush(g_display);
	}
#endif


	XFlush(g_display);
	fflush(stdout);
}

Visual *
x_try_find_visual(int depth, int screen_num, XVisualInfo **visual_list_ptr)
{
	XVisualInfo *visualList;
	XVisualInfo *v_chosen;
	XVisualInfo vTemplate;
	int	visualsMatched;
	int	mdepth;
	int	needs_cmap;
	int	visual_chosen;
	int	match8, match24;
	int	i;

	vTemplate.screen = screen_num;
	vTemplate.depth = depth;

	visualList = XGetVisualInfo(g_display,
		(VisualScreenMask | VisualDepthMask),
		&vTemplate, &visualsMatched);

	vid_printf("visuals matched: %d\n", visualsMatched);
	if(visualsMatched == 0) {
		return (Visual *)0;
	}

	visual_chosen = -1;
	needs_cmap = 0;
	for(i = 0; i < visualsMatched; i++) {
		printf("Visual %d\n", i);
		printf("	id: %08x, screen: %d, depth: %d, class: %d\n",
			(word32)visualList[i].visualid,
			visualList[i].screen,
			visualList[i].depth,
			visualList[i].c_class);
		printf("	red: %08lx, green: %08lx, blue: %08lx\n",
			visualList[i].red_mask,
			visualList[i].green_mask,
			visualList[i].blue_mask);
		printf("	cmap size: %d, bits_per_rgb: %d\n",
			visualList[i].colormap_size,
			visualList[i].bits_per_rgb);
		match8 = (visualList[i].c_class == PseudoColor);
		match24 = (visualList[i].c_class == TrueColor);
		if((depth == 8) && match8) {
			visual_chosen = i;
			Max_color_size = visualList[i].colormap_size;
			needs_cmap = 1;
			break;
		}
		if((depth != 8) && match24) {
			visual_chosen = i;
			Max_color_size = -1;
			needs_cmap = 0;
			break;
		}
	}

	if(visual_chosen < 0) {
		printf("Couldn't find any good visuals at depth %d!\n",
			depth);
		return (Visual *)0;
	}

	printf("Chose visual: %d, max_colors: %d\n", visual_chosen,
		Max_color_size);

	v_chosen = &(visualList[visual_chosen]);
	x_set_mask_and_shift(v_chosen->red_mask, &g_red_mask,
				&g_red_left_shift, &g_red_right_shift);
	x_set_mask_and_shift(v_chosen->green_mask, &g_green_mask,
				&g_green_left_shift, &g_green_right_shift);
	x_set_mask_and_shift(v_chosen->blue_mask, &g_blue_mask,
				&g_blue_left_shift, &g_blue_right_shift);

	g_screen_depth = depth;
	mdepth = depth;
	if(depth > 8) {
		mdepth = 16;
	}
	if(depth > 16) {
		mdepth = 32;
	}
	g_screen_mdepth = mdepth;
	g_needs_cmap = needs_cmap;
	*visual_list_ptr = visualList;

	return v_chosen->visual;
}

void
x_set_mask_and_shift(word32 x_mask, word32 *mask_ptr, int *shift_left_ptr,
		int *shift_right_ptr)
{
	int	shift;
	int	i;

	/* Shift until we find first set bit in mask, then remember mask,shift*/

	shift = 0;
	for(i = 0; i < 32; i++) {
		if(x_mask & 1) {
			/* we're done! */
			break;
		}
		x_mask = x_mask >> 1;
		shift++;
	}
	*mask_ptr = x_mask;
	*shift_left_ptr = shift;
	/* Now, calculate shift_right_ptr */
	shift = 0;
	x_mask |= 1;		// make sure at least one bit is set
	while(x_mask < 0x80) {
		shift++;
		x_mask = x_mask << 1;
	}

	*shift_right_ptr = shift;
	return;

}

int g_xshm_error = 0;

int
xhandle_shm_error(Display *display, XErrorEvent *event)
{
	g_xshm_error = 1;
	return 0;
}

void
x_get_kimage(Kimage *kimage_ptr) {
	if(g_use_shmem) {
		g_use_shmem = get_shm(kimage_ptr);
	}
	if(!g_use_shmem) {
		get_ximage(kimage_ptr);
	}
}

int
get_shm(Kimage *kimage_ptr)
{
#ifdef X_SHARED_MEM
	XShmSegmentInfo *seginfo;
	XImage *xim;
	int	(*old_x_handler)(Display *, XErrorEvent *);
	int	width;
	int	height;
	int	depth;

	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;

	seginfo = (XShmSegmentInfo *)malloc(sizeof(XShmSegmentInfo));
	xim = XShmCreateImage(g_display, g_vis, depth, ZPixmap,
		(char *)0, seginfo, width, height);

	/* check mdepth! */
	if(xim->bits_per_pixel != kimage_ptr->mdepth) {
		printf("get_shm bits_per_pix: %d != %d\n",
				xim->bits_per_pixel, g_screen_mdepth);
	}

	vid_printf("xim: %p\n", xim);
	kimage_ptr->dev_handle = xim;
	kimage_ptr->dev_handle2 = seginfo;
	if(xim == 0) {
		return 0;
	}

	/* It worked, we got it */
	seginfo->shmid = shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height,
		IPC_CREAT | 0777);
	vid_printf("seginfo->shmid = %d\n", seginfo->shmid);
	if(seginfo->shmid < 0) {
		XDestroyImage(xim);
		return 0;
	}

	/* Still working */
	seginfo->shmaddr = (char *)shmat(seginfo->shmid, 0, 0);
	vid_printf("seginfo->shmaddr: %p\n", seginfo->shmaddr);
	if(seginfo->shmaddr == ((char *) -1)) {
		XDestroyImage(xim);
		return 0;
	}

	/* Still working */
	xim->data = seginfo->shmaddr;
	seginfo->readOnly = False;

	/* XShmAttach will trigger X error if server is remote, so catch it */
	g_xshm_error = 0;
	old_x_handler = XSetErrorHandler(xhandle_shm_error);

	XShmAttach(g_display, seginfo);
	XSync(g_display, False);


	vid_printf("about to RMID the shmid\n");
	shmctl(seginfo->shmid, IPC_RMID, 0);

	XFlush(g_display);
	XSetErrorHandler(old_x_handler);

	if(g_xshm_error) {
		XDestroyImage(xim);
		/* We could release the shared mem segment, but by doing the */
		/* RMID, it will go away when we die now, so just leave it */
		printf("Not using shared memory\n");
		return 0;
	}

	kimage_ptr->data_ptr = (byte *)xim->data;
	vid_printf("Sharing memory. xim: %p, xim->data: %p\n", xim, xim->data);

	return 1;
#else
	return 0;		/* No shared memory */
#endif	/* X_SHARED_MEM */
}

void
get_ximage(Kimage *kimage_ptr)
{
	XImage	*xim;
	byte	*ptr;
	int	width;
	int	height;
	int	depth;
	int	mdepth;

	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;
	mdepth = kimage_ptr->mdepth;

	ptr = (byte *)malloc((width * height * mdepth) >> 3);

	vid_printf("ptr: %p\n", ptr);

	if(ptr == 0) {
		printf("malloc for data failed, mdepth: %d\n", mdepth);
		exit(2);
	}

	kimage_ptr->data_ptr = ptr;

	xim = XCreateImage(g_display, g_vis, depth, ZPixmap, 0,
		(char *)ptr, width, height, 8, 0);

#if defined(GSPORT_LITTLE_ENDIAN) || defined (__LITTLE_ENDIAN__) // OSX needs to calculate endianness mid-compilation, can't be passed on compile command
	xim->byte_order = LSBFirst;
#else
	xim->byte_order = MSBFirst;
#endif
	XInitImage(xim);	/* adjust to new byte order */

	/* check mdepth! */
	if(xim->bits_per_pixel != mdepth) {
		printf("shm_ximage bits_per_pix: %d != %d\n",
					xim->bits_per_pixel, mdepth);
	}

	vid_printf("xim: %p\n", xim);

	kimage_ptr->dev_handle = xim;

	return;
}


void
x_redraw_status_lines()
{
	char	*buf;
	int	line;
	int	height;
	int	margin;
	word32	white, black;

	height = g_text_FontSt->ascent + g_text_FontSt->descent;
	margin = g_text_FontSt->ascent;

	white = (g_a2vid_palette << 4) + 0xf;
	black = (g_a2vid_palette << 4) + 0x0;
	if(g_screen_depth != 8) {
		white = (2 << (g_screen_depth - 1)) - 1;
		black = 0;
	}
	XSetForeground(g_display, g_a2_winGC, white);
	XSetBackground(g_display, g_a2_winGC, black);

	for(line = 0; line < MAX_STATUS_LINES; line++) {
		buf = g_status_ptrs[line];
		if(buf == 0) {
			/* skip it */
			continue;
		}
		XDrawImageString(g_display, g_a2_win, g_a2_winGC, 0,
			X_A2_WINDOW_HEIGHT + height*line + margin,
			buf, strlen(buf));
	}

	XFlush(g_display);
}



void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
	int width, int height)
{
	XImage	*xim;

	xim = (XImage *)kimage_ptr->dev_handle;

#ifdef X_SHARED_MEM
	if(g_use_shmem) {
		XShmPutImage(g_display, g_a2_win, g_a2_winGC, xim,
			srcx, srcy, destx, desty, width, height, False);
	}
#endif
	if(!g_use_shmem) {
		XPutImage(g_display, g_a2_win, g_a2_winGC, xim,
			srcx, srcy, destx, desty, width, height);
	}
}

void
x_push_done()
{
	XFlush(g_display);
}


#define KEYBUFLEN	128

int g_num_check_input_calls = 0;
int g_check_input_flush_rate = 2;

int
x_update_mouse(int raw_x, int raw_y, int button_states, int buttons_valid)
{
	int	x, y;

	x = raw_x - BASE_MARGIN_LEFT;
	y = raw_y - BASE_MARGIN_TOP;

	if(g_warp_pointer && (x == A2_WINDOW_WIDTH/2) &&
			(y == A2_WINDOW_HEIGHT/2) && (buttons_valid == 0) ) {
		/* tell adb routs to recenter but ignore this motion */
		update_mouse(x, y, 0, -1);
		return 0;
	}
	return update_mouse(x, y, button_states, buttons_valid & 7);
}

void
check_input_events()
{
	XEvent	ev;
	int	len;
	int	motion;
	int	buttons;
	int	refresh_needed;

	g_num_check_input_calls--;
	if(g_num_check_input_calls < 0) {
		len = XPending(g_display);
		g_num_check_input_calls = g_check_input_flush_rate;
	} else {
		len = QLength(g_display);
	}

	motion = 0;
	refresh_needed = 0;
	while(len > 0) {
		XNextEvent(g_display, &ev);
		len--;
		if(len == 0) {
			/* Xaccel on linux only buffers one X event */
			/*  must look for more now */
			len = XPending(g_display);
		}
		switch(ev.type) {
		case FocusIn:
		case FocusOut:
			if(ev.xfocus.type == FocusOut) {
				/* Allow keyrepeat again! */
				vid_printf("Left window, auto repeat on\n");
				x_auto_repeat_on(0);
				g_has_focus = 0;
			} else if(ev.xfocus.type == FocusIn) {
				/* Allow keyrepeat again! */
				vid_printf("Enter window, auto repeat off\n");
				x_auto_repeat_off(0);
				g_has_focus = 1;
			}
			break;
		case EnterNotify:
		case LeaveNotify:
			/* These events are disabled now */
			printf("Enter/Leave event for winow %08x, sub: %08x\n",
				(word32)ev.xcrossing.window,
				(word32)ev.xcrossing.subwindow);
			printf("Enter/L mode: %08x, detail: %08x, type:%02x\n",
				ev.xcrossing.mode, ev.xcrossing.detail,
				ev.xcrossing.type);
			break;
		case ButtonPress:
			vid_printf("Got button press of button %d!\n",
						ev.xbutton.button);
			buttons = (1 << ev.xbutton.button) >> 1;
			motion |= x_update_mouse(ev.xbutton.x, ev.xbutton.y,
				buttons, buttons & 7);

			break;
		case ButtonRelease:
			buttons = (1 << ev.xbutton.button) >> 1;
			motion |= x_update_mouse(ev.xbutton.x, ev.xbutton.y, 0,
								buttons & 7);
			break;
		case MotionNotify:
			if(ev.xmotion.window != g_a2_win) {
				printf("Motion in window %08x unknown!\n",
					(word32)ev.xmotion.window);
			}
			motion |= x_update_mouse(ev.xmotion.x, ev.xmotion.y, 0,
								0);
			break;
		case Expose:
			refresh_needed = -1;
			break;
		case NoExpose:
			/* do nothing */
			break;
		case KeyPress:
		case KeyRelease:
			handle_keysym(&ev);
			break;
		case KeymapNotify:
			break;
		case ColormapNotify:
			vid_printf("ColormapNotify for %08x\n",
				(word32)(ev.xcolormap.window));
			vid_printf("colormap: %08x, new: %d, state: %d\n",
				(word32)ev.xcolormap.colormap,
				ev.xcolormap.c_new, ev.xcolormap.state);
			break;
		default:
			printf("X event 0x%08x is unknown!\n",
				ev.type);
			break;
		}
	}

	if(motion && g_warp_pointer) {
		XWarpPointer(g_display, None, g_a2_win, 0, 0, 0, 0,
			BASE_MARGIN_LEFT + (A2_WINDOW_WIDTH/2),
			BASE_MARGIN_TOP + (A2_WINDOW_HEIGHT/2));
	}

	if(refresh_needed) {
		printf("Full refresh needed\n");
		g_a2_screen_buffer_changed = -1;
		g_full_refresh_needed = -1;

		g_border_sides_refresh_needed = 1;
		g_border_special_refresh_needed = 1;
		g_status_refresh_needed = 1;

		/* x_refresh_ximage(); */
		/* redraw_border(); */
	}

}

void
x_hide_pointer(int do_hide)
{
	if(do_hide) {
		XDefineCursor(g_display, g_a2_win, g_cursor);
	} else {
		XDefineCursor(g_display, g_a2_win, None);
	}
}


void
handle_keysym(XEvent *xev_in)
{
	KeySym	keysym;
	word32	state;
	int	keycode;
	int	a2code;
	int	type;
	int	is_up;

	keycode = xev_in->xkey.keycode;
	type = xev_in->xkey.type;

	keysym = XLookupKeysym(&(xev_in->xkey), 0);

	state = xev_in->xkey.state;

	vid_printf("keycode: %d, type: %d, state:%d, sym: %08x\n",
		keycode, type, state, (word32)keysym);

	x_update_modifier_state(state);

	is_up = 0;
	if(type == KeyRelease) {
		is_up = 1;
	}

#if 0
	if(keysym == XK_Alt_L || keysym == XK_Meta_L) {
		g_alt_left_up = is_up;
	}

	if(keysym == XK_Alt_R || keysym == XK_Meta_R) {
		g_alt_right_up = is_up;
	}

	if(g_alt_left_up == 0 && g_alt_right_up == 0) {
		printf("Sending sound to file\n");
		g_send_sound_to_file = 1;
	} else {
		if(g_send_sound_to_file) {
			printf("Stopping sending sound to file\n");
			close_sound_file();
		}
		g_send_sound_to_file = 0;
	}
#endif

	/* first, do conversions */
	switch(keysym) {
	case XK_Alt_R:
	case XK_Meta_R:
	case XK_Super_R:
	case XK_Mode_switch:
	case XK_Cancel:
		keysym = XK_Print;		/* option */
		break;
	case XK_Alt_L:
	case XK_Meta_L:
	case XK_Super_L:
	case XK_Menu:
		keysym = XK_Scroll_Lock;	/* cmd */
		break;
	case 0x1000003:
		if(keycode == 0x3c) {
			/* enter key on Mac OS X laptop--make it option */
			keysym = XK_Print;
		}
		break;
	case NoSymbol:
		switch(keycode) {
		/* 94-95 are for my PC101 kbd + windows keys on HPUX */
		case 0x0095:
			/* left windows key = option */
			keysym = XK_Print;
			break;
		case 0x0096:
		case 0x0094:
			/* right windows key = cmd */
			keysym = XK_Scroll_Lock;
			break;
		/* 0072 is for cra@WPI.EDU who says it's Break under XFree86 */
		case 0x0072:
		/* 006e is break according to mic@research.nj.nec.com */
		case 0x006e:
			keysym = XK_Break;
			break;

		/* 0x0042, 0x0046, and 0x0048 are the windows keys according */
		/*  to Geoff Weiss on Solaris x86 */
		case 0x0042:
		case 0x0046:
			/* flying windows == open apple */
			keysym = XK_Scroll_Lock;
			break;
		case 0x0048:
		case 0x0076:		/* Windows menu key on Mac OS X */
			/* menu windows == option */
			keysym = XK_Print;
			break;
		}
	}

	a2code = x_keysym_to_a2code(keysym, is_up);
	if(a2code >= 0) {
		adb_physical_key_update(a2code, is_up);
	} else if(a2code != -2) {
		printf("Keysym: %04x of keycode: %02x unknown\n",
			(word32)keysym, keycode);
	}
}

int
x_keysym_to_a2code(int keysym, int is_up)
{
	int	i;

	if(keysym == 0) {
		return -1;
	}

	if((keysym == XK_Shift_L) || (keysym == XK_Shift_R)) {
		if(is_up) {
			g_x_shift_control_state &= ~ShiftMask;
		} else {
			g_x_shift_control_state |= ShiftMask;
		}
	}
	if(keysym == XK_Caps_Lock) {
		if(is_up) {
			g_x_shift_control_state &= ~LockMask;
		} else {
			g_x_shift_control_state |= LockMask;
		}
	}
	if((keysym == XK_Control_L) || (keysym == XK_Control_R)) {
		if(is_up) {
			g_x_shift_control_state &= ~ControlMask;
		} else {
			g_x_shift_control_state |= ControlMask;
		}
	}

	/* Look up Apple 2 keycode */
	for(i = g_num_a2_keycodes - 1; i >= 0; i--) {
		if((keysym == a2_key_to_xsym[i][1]) ||
					(keysym == a2_key_to_xsym[i][2])) {

			vid_printf("Found keysym:%04x = a[%d] = %04x or %04x\n",
				(int)keysym, i, a2_key_to_xsym[i][1],
				a2_key_to_xsym[i][2]);

			return a2_key_to_xsym[i][0];
		}
	}

	return -1;
}

void
x_update_modifier_state(int state)
{
	int	state_xor;
	int	is_up;

	state = state & (ControlMask | LockMask | ShiftMask);
	state_xor = g_x_shift_control_state ^ state;
	is_up = 0;
	if(state_xor & ControlMask) {
		is_up = ((state & ControlMask) == 0);
		adb_physical_key_update(0x36, is_up);
	}
	if(state_xor & LockMask) {
		is_up = ((state & LockMask) == 0);
		adb_physical_key_update(0x39, is_up);
	}
	if(state_xor & ShiftMask) {
		is_up = ((state & ShiftMask) == 0);
		adb_physical_key_update(0x38, is_up);
	}

	g_x_shift_control_state = state;
}

void
x_auto_repeat_on(int must)
{
	if((g_auto_repeat_on <= 0) || must) {
		g_auto_repeat_on = 1;
		XAutoRepeatOn(g_display);
		XFlush(g_display);
		adb_kbd_repeat_off();
	}
}

void
x_auto_repeat_off(int must)
{
	if((g_auto_repeat_on != 0) || must) {
		XAutoRepeatOff(g_display);
		XFlush(g_display);
		g_auto_repeat_on = 0;
		adb_kbd_repeat_off();
	}
}

void
x_full_screen(int do_full)
{
	return;
}

// OG Adding release
void x_release_kimage(Kimage* kimage_ptr)
{
	if (kimage_ptr->dev_handle == (void*)-1)
	{
		free(kimage_ptr->data_ptr);
		kimage_ptr->data_ptr = NULL;
	}
}

// OG Addding ratio
int x_calc_ratio(float x,float y)
{
	return 1;
}

void
clipboard_paste(void)
{
	// TODO: Add clipboard support
}

int
clipboard_get_char(void)
{
	// TODO: Add clipboard support
	return 0;
}
