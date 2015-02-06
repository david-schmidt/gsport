/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2013 by GSport contributors
 
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

#define WIN32_LEAN_AND_MEAN	/* Tell windows we want less header gunk */
#define STRICT			/* Tell Windows we want compile type checks */

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#include "defc.h"
#include "protos.h"
#include "protos_windriver.h"
#include "win_keymap.h"

extern int Verbose;
extern int g_pause;
extern int g_warp_pointer;
extern int g_screen_depth;
extern Kimage g_mainwin_kimage;
extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];
extern int g_lores_colors[];
extern int g_installed_full_superhires_colormap;
extern int g_config_control_panel;

extern void quitEmulator();
extern void x_toggle_status_lines();
extern void x_show_console(int show);
extern void x_check_input_events();

int	g_win_capslock_down = 0;
int g_screen_mdepth = 0;
int g_num_a2_keycodes = 0;
int g_win_button_states = 0;
int	g_use_shmem = 1;

HDC	g_main_cdc;
HWND	g_hwnd_main;
BITMAPINFO *g_bmapinfo_ptr = 0;
volatile BITMAPINFOHEADER *g_bmaphdr_ptr = 0;

// KEGS32 specific customisations
int		g_win_status_debug = 1;			// Current visibility of status lines.
int		g_win_status_debug_request = 1;	// Desired visibility of status lines.
int		g_win_show_console = 0;			// Current visibility of console.
int		g_win_show_console_request = 1;	// Desired visibility of console.
RECT	g_main_window_saved_rect;
int		g_main_window_saved_style;
int		g_main_window_saved_exstyle;
int		g_win_fullscreen_state = 0;
#if 0 // enable non-integral full-screen scaling
	#define FULLSCREEN_SCALE_TYPE float
#else
	#define FULLSCREEN_SCALE_TYPE int
#endif
FULLSCREEN_SCALE_TYPE	g_win_fullscreen_scale = 1;
int		g_win_fullscreen_offsetx = 0;
int		g_win_fullscreen_offsety = 0;

LPSTR   g_clipboard;
size_t  g_clipboard_pos;

int x_calc_ratio(float ratiox, float ratioy);


int
win_update_mouse(int x, int y, int button_states, int buttons_valid)
{
	int	buttons_changed;

#ifdef ACTIVEGS
	if (g_config_control_panel)	// OG ignore input events while in debugger
		return 0;

	buttons_valid &= 1;	// filter out middle & right button

#endif

	buttons_changed = ((g_win_button_states & buttons_valid) !=
								button_states);
	g_win_button_states = (g_win_button_states & ~buttons_valid) |
			(button_states & buttons_valid);
	if(g_warp_pointer && (x == A2_WINDOW_WIDTH/2) &&
			(y == A2_WINDOW_HEIGHT/2) && (!buttons_changed) ) {
		/* tell adb routs to recenter but ignore this motion */
		update_mouse(x, y, 0, -1);
		return 0;
	}
	return update_mouse(x, y, button_states, buttons_valid & 7);
}


// OG Added asynchronous key & mouse handler
#define ASYNCEVENT

#ifndef ASYNCEVENT
#define WIN_EVENT_MOUSE	win_event_mouse
#define WIN_EVENT_KEY	win_event_key
#else
extern void add_event_mouse(int umsg,WPARAM wParam, LPARAM lParam);
extern void add_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags);
#define WIN_EVENT_MOUSE	add_event_mouse
#define WIN_EVENT_KEY	add_event_key
#endif

struct win32_mouse
{
	int	umsg;
	int wparam;
	int lparam;
};

struct win32_key
{
	UINT raw_vk;
	BOOL down;
	int repeat;
	UINT flags;
};

int last_win32_mouse=0;
int next_win32_mouse=0;

int nb_win32_key=0;

#define MAX_EVENT 1024
struct win32_mouse win32_mouses[MAX_EVENT];
struct win32_key win32_keys[MAX_EVENT];

// OG Push Mouse Event

void add_event_mouse(int umsg,WPARAM wParam, LPARAM lParam)
{
	
#ifdef ACTIVEGS
	if (g_config_control_panel)	// OG ignore input events while in debugger
		return;
#endif

	win32_mouses[next_win32_mouse].umsg = umsg;
	win32_mouses[next_win32_mouse].wparam = wParam;
	win32_mouses[next_win32_mouse].lparam = lParam;
	next_win32_mouse = (next_win32_mouse+1)%MAX_EVENT;

//	ASSERT (next_win32_mouse==last_win32_mouse)
}

// OG Push Key Event

void
add_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags)
{
#ifdef ACTIVEGS
	if (g_config_control_panel)	// OG ignore input events while in debugger 
		return;
#endif

	if (nb_win32_key>=MAX_EVENT) 
		return;

	win32_keys[nb_win32_key].raw_vk = raw_vk;
	win32_keys[nb_win32_key].down = down;
	win32_keys[nb_win32_key].repeat = repeat;
	win32_keys[nb_win32_key].flags = flags;
	
	nb_win32_key++;
}

void
win_event_mouse(int umsg,WPARAM wParam, LPARAM lParam)
{
	POINT	pt;
	word32	flags;
	int	buttons;
	int	x, y;
	int	motion;

	x = LOWORD(lParam);
	y = HIWORD(lParam);


	// OG Reformat the mouse coordinates
	float ratiox = 0.0, ratioy = 0.0;
	if (!g_warp_pointer && x_calc_ratio(ratiox,ratioy))
	{
		x = (int)((float)(x)/ratiox);
		y = (int)((float)(y)/ratioy);
	}

	flags = wParam;
	x  -= BASE_MARGIN_LEFT;
	y -=  BASE_MARGIN_TOP;

	buttons = (flags & 1) +
			(((flags >> 1) & 1) << 2) +
			(((flags >> 4) & 1) << 1);
#if 0
	if (umsg!=WM_MOUSEMOVE)
		printf("Mouse at %d, %d fl: %08x, but: %d\n", x, y, flags, buttons);
#endif
	motion = win_update_mouse(x, y, buttons, 7);

	if(motion && g_warp_pointer) {
		/* move mouse to center of screen */
		pt.x = BASE_MARGIN_LEFT + A2_WINDOW_WIDTH/2;
		pt.y = BASE_MARGIN_TOP + A2_WINDOW_HEIGHT/2;
		ClientToScreen(g_hwnd_main, &pt);
		SetCursorPos(pt.x, pt.y);
	}
}

void
win_event_key(HWND hwnd, UINT raw_vk, BOOL down, int repeat, UINT flags)
{
	word32	vk;
	int	a2code;
	int	is_up;
	int	capslock_down;
	int	i;

	if((flags & 0x4000) && down) {
		/* auto-repeating, just ignore it */
		return;
	}

	vk = raw_vk + (flags & 0x100);
#if 0
	printf("Key event, vk=%04x, down:%d, repeat: %d, flags: %08x\n",
			vk, down, repeat, flags);
#endif

	/* remap a few keys here.. sigh */
	if((vk & 0xff) == VK_APPS) {
		/* remap to command */
		vk = VK_MENU;
	}

	if((vk & 0xff) == VK_CAPITAL) {
		// Windows gives us up-and-down events of the actual key
		//  Use GetKeyState to get the true toggle state, and pass
		//  that on to the adb interface
		capslock_down = GetKeyState(VK_CAPITAL) & 0x01;
		if(capslock_down != g_win_capslock_down) {
			g_win_capslock_down = capslock_down;
			adb_physical_key_update(0x39, !capslock_down);
		}

		return;		// Do no more processing!
	}

	/* search a2key_to_wsym to find wsym in col 1 or 2 */
	i = 0;
	is_up = !down;
	for(i = g_num_a2_keycodes-1; i >= 0; i--) {
		a2code = g_a2_key_to_wsym[i][0];
		if((vk == g_a2_key_to_wsym[i][1]) ||
					(vk == g_a2_key_to_wsym[i][2])) {
			vid_printf("Found vk:%04x = %02x\n", vk, a2code);
			adb_physical_key_update(a2code, is_up);
			return;
		}
	}
	printf("VK: %04x unknown\n", vk);
}

void
win_event_quit(HWND hwnd)
{
	quitEmulator();
}

extern int g_needfullrefreshfornextframe ;

void
win_event_redraw()
{
	g_needfullrefreshfornextframe = 1;
}

LRESULT CALLBACK
win_event_handler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	int numDraggedFiles;
	int szFilename;
	LPTSTR lpszFile;

	switch(umsg) {
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONDBLCLK:	// OG Added dblclk as WM_LBUTTONDOWN
		WIN_EVENT_MOUSE(umsg,wParam, lParam);
		return 0;
	case WM_PAINT:
		win_event_redraw();
		break;
	case WM_DROPFILES:
		numDraggedFiles = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < numDraggedFiles; i++) {
			szFilename = DragQueryFile((HDROP)wParam, i, NULL, 0);
			lpszFile = (LPTSTR)malloc(szFilename + 1);
			szFilename = DragQueryFile((HDROP)wParam, i, lpszFile, szFilename + 1);
			cfg_inspect_maybe_insert_file(lpszFile, 0);
			free(lpszFile);
		}
		DragFinish((HDROP)wParam);
		break;
	}
	switch(umsg) {
		HANDLE_MSG(hwnd, WM_KEYUP, WIN_EVENT_KEY);
		HANDLE_MSG(hwnd, WM_KEYDOWN, WIN_EVENT_KEY);
		HANDLE_MSG(hwnd, WM_SYSKEYUP, WIN_EVENT_KEY);
		HANDLE_MSG(hwnd, WM_SYSKEYDOWN, WIN_EVENT_KEY);
		HANDLE_MSG(hwnd, WM_DESTROY, win_event_quit);
	}

	return DefWindowProc(hwnd, umsg, wParam, lParam);
}

void gsportinit(HWND _hwnd)
{
	g_hwnd_main = _hwnd;
	
	HDC localdc = GetDC(g_hwnd_main);
	SetTextColor(localdc, 0);
	SetBkColor(localdc, 0xFFFFFF);

	g_main_cdc = CreateCompatibleDC(localdc);
	ReleaseDC(g_hwnd_main,localdc);
}

void gsportshut()
{
	if (g_main_cdc)
	{
		DeleteDC(g_main_cdc);
		g_main_cdc = NULL;
	}
	g_hwnd_main = NULL;
}

void
check_input_events()
{
	x_check_input_events();

	// OG Unstack Mouse Events 		
	int i=last_win32_mouse;
	while(i!=next_win32_mouse)
	{
		win_event_mouse(win32_mouses[i].umsg,win32_mouses[i].wparam,win32_mouses[i].lparam);
		int lb = win32_mouses[i].wparam&1;
		i = (i+1)%MAX_EVENT;
		if (lb != (win32_mouses[i].wparam&1))
			break;	// don't send up & down event at the same time
	}
	last_win32_mouse=i;
	
	for(i=0;i<nb_win32_key;i++)
		win_event_key(NULL,win32_keys[i].raw_vk ,win32_keys[i].down,win32_keys[i].repeat ,	win32_keys[i].flags );
	nb_win32_key=0;

	if (g_win_status_debug_request != g_win_status_debug)
		x_toggle_status_lines();
	if (g_win_show_console_request != g_win_show_console)
	{
		g_win_show_console = g_win_show_console_request;
		x_show_console(g_win_show_console_request);
	}
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
	int i;

	for(i = 0; i < 256; i++) {
		printf("%02x: %08x\n", i, g_palette_8to1624[i]);
	}
}

// OG Add function to clear all get_images loaded (dev dependent)
void x_release_kimage(Kimage *kimage_ptr)
{
	if ((int)kimage_ptr->dev_handle != -1)
	{
		DeleteObject(kimage_ptr->dev_handle);
		kimage_ptr->dev_handle = (void*)-1;
	}
	else
	if (kimage_ptr->data_ptr)
	{
		free(kimage_ptr->data_ptr);
		kimage_ptr->data_ptr = NULL;
	}
}

// OG  Free video global memory 
void
xdriver_end()	// Should be renamed to dev_video_shut()  ???
{

	x_release_kimage(&g_mainwin_kimage);
		
	GlobalFree(g_bmapinfo_ptr); // allocated in dev_video_init
	g_bmapinfo_ptr = 0;

	printf("win32 video driver end\n");
}

void
x_get_kimage(Kimage *kimage_ptr)
{
	byte	*ptr;
	int	width;
	int	height;
	int	depth, mdepth;
	int	size;

	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;
	mdepth = kimage_ptr->mdepth;

	size = 0;
	if(depth == g_screen_depth) {
		/* Use g_bmapinfo_ptr, adjusting width, height */
		g_bmaphdr_ptr->biWidth = width;
		g_bmaphdr_ptr->biHeight = -height;

		HDC localdc = GetDC(g_hwnd_main);	// OG Use on the fly DC
		kimage_ptr->dev_handle = CreateDIBSection(localdc,
				g_bmapinfo_ptr, DIB_RGB_COLORS,
				(VOID **)&(kimage_ptr->data_ptr), NULL, 0);
		ReleaseDC(g_hwnd_main,localdc);
	} else {
		/* allocate buffers for video.c to draw into */

		size = (width*height*mdepth) >> 3;
		ptr = (byte *)malloc(size);

		if(ptr == 0) {
			printf("malloc for data failed, mdepth: %d\n", mdepth);
			exit(2);
		}

		kimage_ptr->data_ptr = ptr;

		kimage_ptr->dev_handle = (void *)-1;

	}
	printf("kim: %p, dev:%p data: %p, size: %08x\n", kimage_ptr,
		kimage_ptr->dev_handle, kimage_ptr->data_ptr, size);

	return;
}

void
dev_video_init()
{
	int	extra_size;
	int	lores_col;
	int	i;

	printf("Preparing graphics system\n");

	// OG fix g_num_a2_keycodes identification
	g_num_a2_keycodes = sizeof(g_a2_key_to_wsym)/3/sizeof(int);
	g_num_a2_keycodes--; // last entry=-1

	/*
	g_num_a2_keycodes = 0;
	for(i = 0; i < maxcode; i++) {
		int a2code = g_a2_key_to_wsym[i][0];
		if(a2code < 0) {
			g_num_a2_keycodes = i;
		}
	}
	*/

	// OG Forcing 16bits depth for WiNCE
#ifndef UNDER_CE
	g_screen_depth = 24;
	g_screen_mdepth = 32;
#else
	g_screen_depth = 16;
	g_screen_mdepth = 16;

	
extern word32	g_red_mask ;
extern word32	g_green_mask ;
extern word32	g_blue_mask ;
extern int	g_red_left_shift;
extern int	g_green_left_shift;
extern int	g_blue_left_shift ;
extern int	g_red_right_shift ;
extern int	g_green_right_shift ;
extern int	g_blue_right_shift ;


	g_red_mask = 0xff;
	g_green_mask = 0xff;
	g_blue_mask = 0xff;
	g_red_left_shift = 10;
	g_green_left_shift = 5;
	g_blue_left_shift = 0;
	g_red_right_shift = 3;
	g_green_right_shift = 3;
	g_blue_right_shift = 3;
#endif
	
	extra_size = sizeof(RGBQUAD);
	if(g_screen_depth == 8) {
		extra_size = 256 * sizeof(RGBQUAD);
	}
	g_bmapinfo_ptr = (BITMAPINFO *)GlobalAlloc(GPTR,
			sizeof(BITMAPINFOHEADER) + extra_size);

	g_bmaphdr_ptr = (BITMAPINFOHEADER *)g_bmapinfo_ptr;
	g_bmaphdr_ptr->biSize = sizeof(BITMAPINFOHEADER);
	g_bmaphdr_ptr->biWidth = A2_WINDOW_WIDTH;
	g_bmaphdr_ptr->biHeight = -A2_WINDOW_HEIGHT;
	g_bmaphdr_ptr->biPlanes = 1;
	g_bmaphdr_ptr->biBitCount = g_screen_mdepth;
	g_bmaphdr_ptr->biCompression = BI_RGB;
	g_bmaphdr_ptr->biClrUsed = 0;

	video_get_kimages();

	if(g_screen_depth != 8) {
		//	Allocate g_mainwin_kimage
		video_get_kimage(&g_mainwin_kimage, 0, g_screen_depth,
						g_screen_mdepth);
	}

	for(i = 0; i < 256; i++) {
		lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}

	g_installed_full_superhires_colormap = 1;

#ifndef UNDER_CE
	ShowWindow(g_hwnd_main, SW_SHOWDEFAULT);
#else
	ShowWindow(g_hwnd_main, SW_SHOW);
#endif
	UpdateWindow(g_hwnd_main);

	printf("Done with dev_video_init\n");
	fflush(stdout);
}

void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
	int width, int height)
{
	void	*bitm_old;

	HDC localdc = GetDC(g_hwnd_main);
	HDC localcdc = g_main_cdc;

	bitm_old = SelectObject(localcdc, kimage_ptr->dev_handle);

	if (g_win_fullscreen_scale == 1)
		BitBlt(localdc, destx + g_win_fullscreen_offsetx, desty + g_win_fullscreen_offsety, width, height, localcdc, srcx, srcy, SRCCOPY);
	else
	{
		FULLSCREEN_SCALE_TYPE xdest = destx * g_win_fullscreen_scale + g_win_fullscreen_offsetx;
		FULLSCREEN_SCALE_TYPE ydest = desty * g_win_fullscreen_scale + g_win_fullscreen_offsety;
		FULLSCREEN_SCALE_TYPE wdest = width * g_win_fullscreen_scale;
		FULLSCREEN_SCALE_TYPE hdest = height * g_win_fullscreen_scale;

		StretchBlt(localdc, (int)xdest, (int)ydest, (int)wdest, (int)hdest, localcdc, srcx, srcy, width, height, SRCCOPY);
	}

	SelectObject(localcdc, bitm_old);
	ReleaseDC(g_hwnd_main,localdc);
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
	if(do_hide) {
		ShowCursor(0);
	} else {
		ShowCursor(1);
	}
}

void
x_full_screen(int do_full)
{
	MONITORINFO monitor_info;
	FULLSCREEN_SCALE_TYPE width, height, scalex, scaley;
	int top, left;

	g_win_status_debug = 1 - do_full;
	if (do_full && !g_win_fullscreen_state) {
		g_main_window_saved_style = GetWindowLong(g_hwnd_main, GWL_STYLE);
		g_main_window_saved_exstyle = GetWindowLong(g_hwnd_main, GWL_EXSTYLE);
		GetWindowRect(g_hwnd_main, &g_main_window_saved_rect);
		SetWindowLong(g_hwnd_main, GWL_STYLE, g_main_window_saved_style & ~(WS_CAPTION | WS_THICKFRAME));
		SetWindowLong(g_hwnd_main, GWL_EXSTYLE, g_main_window_saved_exstyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

		monitor_info.cbSize = sizeof(monitor_info);
		GetMonitorInfo(MonitorFromWindow(g_hwnd_main, MONITOR_DEFAULTTONEAREST), &monitor_info);
		left = monitor_info.rcMonitor.left;
		top = monitor_info.rcMonitor.top;
		width = (FULLSCREEN_SCALE_TYPE)(monitor_info.rcMonitor.right - monitor_info.rcMonitor.left);
		height = (FULLSCREEN_SCALE_TYPE)(monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top);
		scalex = width / X_A2_WINDOW_WIDTH;
		scaley = height / X_A2_WINDOW_HEIGHT;
		g_win_fullscreen_scale = (scalex <= scaley) ? scalex : scaley;
		g_win_fullscreen_offsetx = ((int)width - (int)(g_win_fullscreen_scale * X_A2_WINDOW_WIDTH)) / 2;
		g_win_fullscreen_offsety = ((int)height - (int)(g_win_fullscreen_scale * X_A2_WINDOW_HEIGHT)) / 2;
		SetWindowPos(g_hwnd_main, NULL, left, top, (int)width, (int)height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		g_win_fullscreen_state = 1;
		RedrawWindow(g_hwnd_main, NULL, NULL, RDW_INVALIDATE);


	} else {
		if (g_win_fullscreen_state) {
			g_win_fullscreen_offsetx = 0;
			g_win_fullscreen_offsety = 0;
			g_win_fullscreen_scale = 1;
			SetWindowLong(g_hwnd_main, GWL_STYLE, g_main_window_saved_style);
			SetWindowLong(g_hwnd_main, GWL_EXSTYLE, g_main_window_saved_exstyle);
			SetWindowPos(g_hwnd_main, NULL,
				g_main_window_saved_rect.left,
				g_main_window_saved_rect.top,
				g_main_window_saved_rect.right - g_main_window_saved_rect.left,
				g_main_window_saved_rect.bottom - g_main_window_saved_rect.top,
				SWP_SHOWWINDOW);

			g_win_fullscreen_state = 0;
			RedrawWindow(g_hwnd_main, NULL, NULL, RDW_INVALIDATE);
		}
	}
	return;
}

void
clipboard_paste()
{
	if (!IsClipboardFormatAvailable(CF_TEXT))
		return;
	if (!OpenClipboard(g_hwnd_main))
		return;
	{
		HGLOBAL hclipboard = GetClipboardData(CF_TEXT);
		if (!hclipboard) {
			CloseClipboard();
			return;
		}
		{
			LPSTR clipboard = (LPSTR)GlobalLock(hclipboard);
			if (!clipboard) {
				CloseClipboard();
				return;
			}
			if (g_clipboard) {
				free(g_clipboard);
				g_clipboard_pos = 0;
			}
			g_clipboard = strdup(clipboard);
		}
		GlobalUnlock(hclipboard);
	}
	CloseClipboard();
}

int
clipboard_get_char()
{
	if (!g_clipboard)
		return 0;
	if (g_clipboard[g_clipboard_pos] == '\n')
		g_clipboard_pos++;
	if (g_clipboard[g_clipboard_pos] == '\0')
		return 0;
	return g_clipboard[g_clipboard_pos++] | 0x80;
}
