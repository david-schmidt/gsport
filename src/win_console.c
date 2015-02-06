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
#include <Shlwapi.h>

#include "winresource.h"
#include "defc.h"
#include "protos_windriver.h"


extern void gsportinit(HWND _hwnd);
extern void gsportshut();
extern HWND g_hwnd_main;

extern char *g_status_ptrs[MAX_STATUS_LINES];
extern int  g_win_status_debug;
extern int  g_win_status_debug_request;
extern int  g_win_fullscreen_state;

int
win_nonblock_read_stdin(int fd, char *bufptr, int len)
{
	DWORD charsRead = 0;
	ReadConsole(GetStdHandle(STD_INPUT_HANDLE), bufptr, len, &charsRead, NULL);
	
	if (charsRead == 0)
	{
		errno = EAGAIN;
		return -1;
	}
	else
	{
		DWORD charsWritten = 0;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), bufptr, charsRead, &charsWritten, NULL);
		return charsRead;
	}
}

void get_cwd(LPTSTR buffer, int size)
{
	HMODULE hSelf;
	hSelf = GetModuleHandle(NULL);
	GetModuleFileName(hSelf,buffer,size);
	PathRemoveFileSpec(buffer);
	printf("Local directory: [%s]\n",buffer);
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
	return 0;
}

void get_default_window_size(LPSIZE size)
{
	// Calculate the window client dimensions.
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.bottom = X_A2_WINDOW_HEIGHT;
	if (g_win_status_debug) 
		rect.bottom += (MAX_STATUS_LINES * 16);
	rect.right = X_A2_WINDOW_WIDTH;

	// Calculate the window rectangle, which is the client area plus non-client area (e.g. frame and caption).
	AdjustWindowRect(&rect, WS_TILED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
	
	// Return the window size.
	size->cx = rect.right - rect.left;
	size->cy = rect.bottom - rect.top;
}

void x_toggle_status_lines()
{
	SIZE size;

	if (!g_win_fullscreen_state)
	{
		g_win_status_debug = !g_win_status_debug;
		g_win_status_debug_request = g_win_status_debug;

		get_default_window_size(&size);
		SetWindowPos(g_hwnd_main, NULL, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER);
		x_redraw_status_lines();
	}
}

void x_show_console(int show)
{
	HWND hWnd = GetConsoleWindow();
	if (hWnd)
		ShowWindow(hWnd, show ? SW_SHOW : SW_HIDE);

	if (g_hwnd_main)
		SetFocus(g_hwnd_main);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return main(0,0);
}

int
main(int argc, char **argv)
{
	// Hide the console initially to reduce window flashing.  We'll show the console later if needed.
	x_show_console(0);

	// Register the window class.
	WNDCLASS wndclass;
	SIZE	size;
	RECT	rect;

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)win_event_handler;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(NULL);
	wndclass.hIcon = LoadIcon(wndclass.hInstance, MAKEINTRESOURCE(IDC_GSPORT32));
	wndclass.hCursor = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "gsport";

	if(!RegisterClass(&wndclass)) {
		printf("Registering window failed\n");
		exit(1);
	}

	// Create the window.
	get_default_window_size(&size);
	
	HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, "gsport", "GSport - Apple //gs Emulator",
		WS_TILED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		size.cx, size.cy,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	printf("g_hwnd_main = %p, height = %d\n", hwnd, size.cx);
	GetWindowRect(hwnd, &rect);
	printf("...rect is: %ld, %ld, %ld, %ld\n", rect.left, rect.top,
		rect.right, rect.bottom);
	
	// Enable non-blocking, character-at-a-time console I/O.
	// win_nonblock_read_stdin() expects this behavior.
	DWORD mode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
	mode &= ~ENABLE_LINE_INPUT;
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);


	gsportinit(hwnd);
	int ret =  gsportmain(argc, argv);
	
	UnregisterClass(wndclass.lpszClassName,GetModuleHandle(NULL));

	gsportshut();
	return ret;
}

void x_check_input_events()
{
	MSG	msg;

	while(PeekMessage(&msg, g_hwnd_main, 0, 0, PM_NOREMOVE)) {
		if(GetMessage(&msg, g_hwnd_main, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			printf("GetMessage returned <= 0\n");
			my_exit(2);
		}
	}
}

void
x_redraw_status_lines()
{
	COLORREF oldtextcolor, oldbkcolor;
	char	*buf;
	int	line;
	int	len;
	int	height;
	int	margin;

	height = 16;
	margin = 0;
	if (g_win_status_debug)
	{
		HDC localdc = GetDC(g_hwnd_main);
		oldtextcolor = SetTextColor(localdc, 0);
		oldbkcolor = SetBkColor(localdc, 0xffffff);
		for(line = 0; line < MAX_STATUS_LINES; line++) {
			buf = g_status_ptrs[line];
			if(buf != 0) {
				len = strlen(buf);
				TextOut(localdc, 10, X_A2_WINDOW_HEIGHT +
					height*line + margin, buf, len);
			}
		}
		SetTextColor(localdc, oldtextcolor);
		SetBkColor(localdc, oldbkcolor);
		ReleaseDC(g_hwnd_main,localdc);
	}
}

int
x_calc_ratio(float ratiox,float ratioy)
{
	return 0; // not stretched
}