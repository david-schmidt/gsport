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

#include <windows.h>
#include <windowsx.h>
#include <Shlwapi.h>
#include <mmsystem.h>
#include <winsock.h>
//#include <commctrl.h>

#include "defc.h"
#include "protos_windriver.h"


extern void gsportinit(HWND _hwnd);
extern void gsportshut();
extern HWND g_hwnd_main;

extern char *g_status_ptrs[MAX_STATUS_LINES];
extern int  g_win_status_debug;

int
win_nonblock_read_stdin(int fd, char *bufptr, int len)
{
	

	HANDLE	oshandle;
	DWORD	dwret;
	int	ret;

	errno = EAGAIN;
	oshandle = (HANDLE)_get_osfhandle(fd);	// get stdin handle
	dwret = WaitForSingleObject(oshandle, 1);	// wait 1msec for data
	ret = -1;
	if(dwret == WAIT_OBJECT_0) {
		ret = read(fd, bufptr, len);
	}
	return ret;

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

int WINAPI WinMain (
	HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
     int nShowCmd)
{
	return main(0,0);
}

int
main(int argc, char **argv)
{
//	InitCommonControls();

	WNDCLASS wndclass;
	RECT	rect;
	int	height;


	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)win_event_handler;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(NULL);
	wndclass.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // OG Added cast
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "gsport";

	// Register the window
	if(!RegisterClass(&wndclass)) {
		printf("Registering window failed\n");
		exit(1);
	}

	height = X_A2_WINDOW_HEIGHT + (MAX_STATUS_LINES * 16) + 32;
//	g_main_height = height;

	
	HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, "gsport", "GSport - Apple //gs Emulator",
		WS_TILED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		X_A2_WINDOW_WIDTH, height,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	printf("g_hwnd_main = %p, height = %d\n", hwnd, height);
	GetWindowRect(hwnd, &rect);
	printf("...rect is: %ld, %ld, %ld, %ld\n", rect.left, rect.top,
		rect.right, rect.bottom);
	
	gsportinit(hwnd);
	int ret =  gsportmain(argc, argv);
	
	UnregisterClass(wndclass.lpszClassName,GetModuleHandle(NULL));

	gsportshut();
	return ret;
}

void
x_check_input_events()
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
	HDC localdc = GetDC(g_hwnd_main);	// OG Use on the fly DC
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

int x_calc_ratio(float ratiox,float ratioy)
{
	return 0; // not stretched
}
