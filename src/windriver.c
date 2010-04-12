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

const char rcsid_windriver_c[] = "@(#)$KmKId: windriver.c,v 1.11 2004-11-24 16:43:46-05 kentd Exp $";

/* Based on code from Chea Chee Keong from KEGS32, which is available at */
/*  http://www.geocities.com/akilgard/kegs32 */

#define WIN32_LEAN_AND_MEAN	/* Tell windows we want less header junk */
#define STRICT			/* Tell Windows we want compile type checks */

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <winsock.h>
#include <commctrl.h>
#include <Commdlg.h>
#include <tchar.h>
#include <shellapi.h>
#include "winresource.h"
#include "defc.h"
#include "protos_windriver.h"
#include "printer.h"


#if defined _MSC_VER && defined _WIN64
#define gwlproc GWLP_WNDPROC
#endif
#if defined _MSC_VER && defined _WIN32 && !defined _WIN64 
#define gwlproc GWL_WNDPROC
#endif

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

HDC	g_main_dc;
HDC	g_main_cdc;
int	g_main_height = 0;

int	g_win_capslock_down = 0;

extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

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

HWND	g_hwnd_main;
BITMAPINFO *g_bmapinfo_ptr = 0;
volatile BITMAPINFOHEADER *g_bmaphdr_ptr = 0;

// KEGS32 specific customisations
HWND    g_win_toolbar=NULL;
HWND    g_win_status=NULL;
RECT    g_win_status_height={0};
RECT    g_win_toolbar_height={0};
int     g_win_status_debug = 0;
RECT	g_main_window_saved_rect;
HMENU	g_main_window_menu_saved;
int	g_win_fullscreen_state = 0;
WNDPROC oldEditWndProc;
extern int g_joystick_type;
extern int g_config_kegs_update_needed;
extern int g_limit_speed;
extern int g_doc_vol;

int g_num_a2_keycodes = 0;

extern char *g_status_ptrs[MAX_STATUS_LINES];

int g_win_button_states = 0;


/* this table is used to search for the Windows VK_* in col 1 or 2 */
/* flags bit 8 is or'ed into the VK, so we can distinguish keypad keys */
/* regardless of numlock */
int g_a2_key_to_wsym[][3] = {
	{ 0x35,	VK_ESCAPE,	0 },
	{ 0x7a,	VK_F1,	0 },
	{ 0x78,	VK_F2,	0 },
	{ 0x63,	VK_F3,	0 },
	{ 0x76,	VK_F4,	0 },
	{ 0x60,	VK_F5,	0 },
	{ 0x61,	VK_F6,	0 },
	{ 0x62,	VK_F7,	0 },
	{ 0x64,	VK_F8,	0 },
	{ 0x65,	VK_F9,	0 },
	{ 0x6d,	VK_F10,	0 },
	{ 0x67,	VK_F11,	0 },
	{ 0x6f,	VK_F12,	0 },
	{ 0x69,	VK_F13,	0 },
	{ 0x6b,	VK_F14,	0 },
	{ 0x71,	VK_F15,	0 },
	{ 0x7f, VK_PAUSE, VK_CANCEL+0x100 },

	{ 0x32,	0xc0, 0 },		/* '`' */
	{ 0x12,	'1', 0 },
	{ 0x13,	'2', 0 },
	{ 0x14,	'3', 0 },
	{ 0x15,	'4', 0 },
	{ 0x17,	'5', 0 },
	{ 0x16,	'6', 0 },
	{ 0x1a,	'7', 0 },
	{ 0x1c,	'8', 0 },
	{ 0x19,	'9', 0 },
	{ 0x1d,	'0', 0 },
	{ 0x1b,	0xbd, 0 },		/* '-' */
	{ 0x18,	0xbb, 0 },		/* '=' */
	{ 0x33,	VK_BACK, 0 },		/* backspace */
	{ 0x72,	VK_INSERT+0x100, 0 },	/* Insert key */
/*	{ 0x73,	XK_Home, 0 },		alias VK_HOME to be KP_Equal! */
	{ 0x74,	VK_PRIOR+0x100, 0 },	/* pageup */
	{ 0x47,	VK_NUMLOCK, VK_NUMLOCK+0x100 },	/* clear */
	{ 0x51,	VK_HOME+0x100, 0 },		/* KP_equal is HOME key */
	{ 0x4b,	VK_DIVIDE, VK_DIVIDE+0x100 },
	{ 0x43,	VK_MULTIPLY, VK_MULTIPLY+0x100 },

	{ 0x30,	VK_TAB, 0 },
	{ 0x0c,	'Q', 0 },
	{ 0x0d,	'W', 0 },
	{ 0x0e,	'E', 0 },
	{ 0x0f,	'R', 0 },
	{ 0x11,	'T', 0 },
	{ 0x10,	'Y', 0 },
	{ 0x20,	'U', 0 },
	{ 0x22,	'I', 0 },
	{ 0x1f,	'O', 0 },
	{ 0x23,	'P', 0 },
	{ 0x21,	0xdb, 0 },	/* [ */
	{ 0x1e,	0xdd, 0 },	/* ] */
	{ 0x2a,	0xdc, 0 },	/* backslash, bar */
	{ 0x75,	VK_DELETE+0x100, 0 },
	{ 0x77,	VK_END+0x100, VK_END },
	{ 0x79,	VK_NEXT+0x100, 0 },
	{ 0x59,	VK_NUMPAD7, VK_HOME },
	{ 0x5b,	VK_NUMPAD8, VK_UP },
	{ 0x5c,	VK_NUMPAD9, VK_PRIOR },
	{ 0x4e,	VK_SUBTRACT, VK_SUBTRACT+0x100 },

	// { 0x39,	VK_CAPITAL, 0 },  // Handled specially!
	{ 0x00,	'A', 0 },
	{ 0x01,	'S', 0 },
	{ 0x02,	'D', 0 },
	{ 0x03,	'F', 0 },
	{ 0x05,	'G', 0 },
	{ 0x04,	'H', 0 },
	{ 0x26,	'J', 0 },
	{ 0x28,	'K', 0 },
	{ 0x25,	'L', 0 },
	{ 0x29,	0xba, 0 },	/* ; */
	{ 0x27,	0xde, 0 },	/* single quote */
	{ 0x24,	VK_RETURN, 0 },
	{ 0x56,	VK_NUMPAD4, VK_LEFT },
	{ 0x57,	VK_NUMPAD5, VK_CLEAR },
	{ 0x58,	VK_NUMPAD6, VK_RIGHT },
	{ 0x45,	VK_ADD, 0 },

	{ 0x38,	VK_SHIFT, 0 },
	{ 0x06,	'Z', 0 },
	{ 0x07,	'X', 0 },
	{ 0x08,	'C', 0 },
	{ 0x09,	'V', 0 },
	{ 0x0b,	'B', 0 },
	{ 0x2d,	'N', 0 },
	{ 0x2e,	'M', 0 },
	{ 0x2b,	0xbc, 0 },	/* , */
	{ 0x2f,	0xbe, 0 },	/* . */
	{ 0x2c,	0xbf, 0 },	/* / */
	{ 0x3e,	VK_UP+0x100, 0 },
	{ 0x53,	VK_NUMPAD1, VK_END },
	{ 0x54,	VK_NUMPAD2, VK_DOWN },
	{ 0x55,	VK_NUMPAD3, VK_NEXT },

	{ 0x36,	VK_CONTROL, VK_CONTROL+0x100 },
	{ 0x3a,	VK_SNAPSHOT+0x100, VK_MENU+0x100 },/* Opt=prntscrn or alt-r */
	{ 0x37,	VK_SCROLL, VK_MENU },		/* Command=scr_lock or alt-l */
	{ 0x31,	' ', 0 },
	{ 0x3b,	VK_LEFT+0x100, 0 },
	{ 0x3d,	VK_DOWN+0x100, 0 },
	{ 0x3c,	VK_RIGHT+0x100, 0 },
	{ 0x52,	VK_NUMPAD0, VK_INSERT },
	{ 0x41,	VK_DECIMAL, VK_DECIMAL },
	{ 0x4c,	VK_RETURN+0x100, 0 },
	{ -1, -1, -1 }
};

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

void
init_window(HWND hwnd,BOOL initFlag) {
        RECT rect;
        RECT wrect;
        int  adjx,adjy;
		int win_height;
        GetClientRect(hwnd,&rect);
        GetWindowRect(hwnd,&wrect);
        adjx=(wrect.right-wrect.left)-(rect.right-rect.left);
        adjy=(wrect.bottom-wrect.top)-(rect.bottom-rect.top);

		win_height=X_A2_WINDOW_HEIGHT+(g_win_toolbar_height.bottom-g_win_toolbar_height.top)+(g_win_status_height.bottom-g_win_status_height.top);

        if (g_win_status_debug) {
                win_height+=(MAX_STATUS_LINES*16);
        } 

        if (initFlag) {
                SetWindowPos(hwnd,NULL,
                             g_main_window_saved_rect.left,
			     g_main_window_saved_rect.top,
                             X_A2_WINDOW_WIDTH+adjx,
                             win_height+adjy,
                             SWP_NOACTIVATE | SWP_NOZORDER);
        } else {
                SetWindowPos(hwnd,HWND_NOTOPMOST,
                             g_main_window_saved_rect.left,
			     g_main_window_saved_rect.top,
                             X_A2_WINDOW_WIDTH+adjx,
                             win_height+adjy,
                             SWP_SHOWWINDOW);
        }
        SendMessage(g_win_status,WM_SIZE,0,0);
        SendMessage(g_win_toolbar,WM_SIZE,0,0);
}

void win_center_dialog (HWND hDlg) {
    RECT rc, rcDlg,rcOwner;
    HWND hwndOwner;

    if ((hwndOwner = GetParent(hDlg)) == NULL) {
        hwndOwner = GetDesktopWindow();
    }

    GetWindowRect(hwndOwner,&rcOwner);
    GetWindowRect(hDlg,&rcDlg);
    CopyRect(&rc,&rcOwner);
    OffsetRect(&rcDlg,-rcDlg.left,-rcDlg.top);
    OffsetRect(&rc,-rc.left,-rc.top);
    OffsetRect(&rc,-rcDlg.right,-rcDlg.bottom);

    SetWindowPos(hDlg,g_hwnd_main,rcOwner.left+(rc.right/2),
                 rcOwner.top+(rc.bottom/2), 0,0,
                 SWP_SHOWWINDOW | SWP_NOSIZE );
}

void
read_disk_entry(HWND hDlg)
{
	char buf[2048];
	int slot,drive;
	HWND hwnd;
	for (slot=5;slot<=7;slot++) {
		for(drive=0;drive<=1;drive++) {
			Disk* dsk = cfg_get_dsk_from_slot_drive(slot,drive);
			buf[0]=0;
			if (dsk->name_ptr && dsk->fd > 0) {
               			hwnd = GetDlgItem(hDlg,10000+slot*10+(drive+1));
				strcpy(buf,dsk->name_ptr);
                       		SetWindowText(hwnd,buf);
			}
		}
	}
}

void
write_disk_entry(HWND hDlg)
{
	char buf[2048];
	int slot,drive;
	HWND hwnd;
	for (slot=5;slot<=7;slot++) {
		for(drive=0;drive<=1;drive++) {
               		hwnd = GetDlgItem(hDlg,10000+slot*10+(drive+1));
			buf[0]=0;
                	GetWindowText(hwnd,buf,2048);
			if (lstrlen(buf)>0) {
				insert_disk(slot, drive, buf, 0, 0, 0, -1);
			} else {
				eject_disk_by_num(slot,drive);
			}
		}
	}
}

// Message handler for handling edit control
// For dropping files
LRESULT CALLBACK win_dialog_edit(HWND hwnd, UINT message, WPARAM wParam, 
                                 LPARAM lParam) {
    switch (message) {
    case WM_DROPFILES:
        {    
            TCHAR buffer[2048]={0};
            HDROP hdrop = (HDROP) wParam;
            memset(buffer,0,2048*sizeof(TCHAR));
            DragQueryFile(hdrop,0,buffer,2048);
            SetWindowText(hwnd,buffer);
            DragFinish(hdrop);
            return 0;
        }
    default:
        return CallWindowProc(oldEditWndProc,hwnd,message,wParam,lParam);
    }
}

// Message handler for disk config.
LRESULT CALLBACK win_dialog_disk(HWND hDlg, UINT message, WPARAM wParam, 
                                 LPARAM lParam)
{
    typedef LONG(CALLBACK *SETWL)(HWND,int,LONG);

    SETWL SetWL=NULL;

    switch (message) {
    case WM_INITDIALOG:
          win_center_dialog(hDlg);

          // Subclass the edit-control
          SetWL=SetWindowLong;
          oldEditWndProc=(WNDPROC) SetWL(GetDlgItem(hDlg,IDC_EDIT_S5D1),
                                         gwlproc,(LONG)win_dialog_edit);
          SetWL(GetDlgItem(hDlg,IDC_EDIT_S5D2),gwlproc,
               (LONG)win_dialog_edit);
          SetWL(GetDlgItem(hDlg,IDC_EDIT_S6D1),gwlproc,
               (LONG)win_dialog_edit);
          SetWL(GetDlgItem(hDlg,IDC_EDIT_S6D2),gwlproc,
               (LONG)win_dialog_edit);
          SetWL(GetDlgItem(hDlg,IDC_EDIT_S7D1),gwlproc,
               (LONG)win_dialog_edit);
          SetWL(GetDlgItem(hDlg,IDC_EDIT_S7D2),gwlproc,
               (LONG)win_dialog_edit);

          // Clean entry field (init)
          SetWindowText(GetDlgItem(hDlg,IDC_EDIT_S5D1),_T(""));
          SetWindowText(GetDlgItem(hDlg,IDC_EDIT_S5D2),_T(""));
          SetWindowText(GetDlgItem(hDlg,IDC_EDIT_S6D1),_T(""));
          SetWindowText(GetDlgItem(hDlg,IDC_EDIT_S6D2),_T(""));
          SetWindowText(GetDlgItem(hDlg,IDC_EDIT_S7D1),_T(""));
          SetWindowText(GetDlgItem(hDlg,IDC_EDIT_S7D2),_T(""));
          read_disk_entry(hDlg);
          return TRUE;
    case WM_COMMAND:
       switch (LOWORD(wParam)) {
       case IDOK:
           write_disk_entry(hDlg);
           EndDialog(hDlg, LOWORD(wParam));
           return TRUE;
       case IDCANCEL:
           EndDialog(hDlg, LOWORD(wParam));
           return FALSE;
       case IDC_BTN_S5D1:
       case IDC_BTN_S5D2:
       case IDC_BTN_S6D1:
       case IDC_BTN_S6D2:
       case IDC_BTN_S7D1:
       case IDC_BTN_S7D2:
           {
               
               TCHAR filename[2048]={0};
             
	       OPENFILENAME opfn;
	       ZeroMemory(&opfn,sizeof(opfn));
	       opfn.lStructSize=sizeof(opfn);
	       opfn.hwndOwner=hDlg;
	       opfn.lpstrFilter=_T("2mg format (*.2mg)\0*.2mg\0"
				   "Prodos order format (*.po)\0*.po\0"
				   "Dos order format (*.dsk)\0*.dsk\0"
				   "All Files (*.*)\0*.*\0"
				   "\0\0");
	       opfn.lpstrFile=filename;
	       opfn.nMaxFile=2048;
	       opfn.Flags=OFN_EXPLORER | OFN_FILEMUSTEXIST | 
			  OFN_HIDEREADONLY | OFN_NOCHANGEDIR ;
	       if (GetOpenFileName(&opfn)) {
		   SetWindowText(GetDlgItem(hDlg,LOWORD(wParam-1000)),
				 filename);
	       }    
           }
           break;
       }
       break;
    }
    return FALSE;
}



// Message handler for about box.
LRESULT CALLBACK win_dialog_about_dialog(HWND hDlg, 
                                         UINT message, WPARAM wParam, 
                                         LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
          win_center_dialog(hDlg);
          return TRUE;
    case WM_COMMAND:
       switch (LOWORD(wParam)) {
       case IDOK:
           EndDialog(hDlg, LOWORD(wParam));
           return TRUE;
       case IDCANCEL:
           EndDialog(hDlg, LOWORD(wParam));
           return FALSE;

       }
    }
    return FALSE;
}

void
x_dialog_create_kegs_conf(const char *str)
{
}

int
x_show_alert(int is_fatal, const char *str)
{
	return 0;
}

int
win_update_mouse(int x, int y, int button_states, int buttons_valid)
{
	int	buttons_changed;

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

void
win_event_mouse(WPARAM wParam, LPARAM lParam)
{
	POINT	pt;
	word32	flags;
	int	buttons;
	int	x, y;
	int	motion;

	flags = wParam;
	x = LOWORD(lParam) - BASE_MARGIN_LEFT;
	y = HIWORD(lParam) - BASE_MARGIN_TOP;

	buttons = (flags & 1) +
			(((flags >> 1) & 1) << 2) +
			(((flags >> 4) & 1) << 1);
#if 0
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
win_event_command(HWND hwnd, int id, HWND g_wnd_ctl, UINT code_notify)
{
        switch (id) {
        case ID_HELP_ABOUT:
                DialogBoxParam(GetModuleHandle(NULL), 
                               (LPCTSTR)IDD_ABOUT_DIALOG,
                               hwnd, 
                               (DLGPROC)win_dialog_about_dialog,0);
                break;
        case ID_HELP_KEY:
                DialogBoxParam(GetModuleHandle(NULL), 
                               (LPCTSTR)IDD_KEGS32_KEY,
                               hwnd, 
                               (DLGPROC)win_dialog_about_dialog,0);
                break;
        case ID_FILE_EXIT:
                PostQuitMessage(0);
                break;

        case ID_FILE_SENDRESET:
                // Simulate key pressing to send reset
                adb_physical_key_update(0x36, 0);
                adb_physical_key_update(0x7f, 0);
                adb_physical_key_update(0x7f, 1);
                adb_physical_key_update(0x36, 1);
                break;
        case ID_FILE_SENDREBOOT:
                // Simulate key pressing to send reset
                adb_physical_key_update(0x36, 0);
                adb_physical_key_update(0x37, 0);
				adb_physical_key_update(0x7f, 0);
                adb_physical_key_update(0x7f, 1);
                adb_physical_key_update(0x36, 1);
                adb_physical_key_update(0x37, 1);
				adb_physical_key_update(0x37, 0);
                break;
		case ID_FILE_FLUSHPRINTER:
				printer_feed();
				break;
        case ID_FILE_DEBUGSTAT:
                g_win_status_debug = ! g_win_status_debug;
		GetWindowRect(g_hwnd_main,&g_main_window_saved_rect);
                init_window(g_hwnd_main,FALSE);
                if (g_win_status_debug) {
                        CheckMenuItem(GetMenu(g_hwnd_main),
                                      ID_FILE_DEBUGSTAT,MF_CHECKED);
                } else {
                        CheckMenuItem(GetMenu(g_hwnd_main),
                                      ID_FILE_DEBUGSTAT,MF_UNCHECKED);
                } 
                break;
   	case ID_FILE_JOYSTICK:
		if (g_joystick_type == 4) { 
			g_joystick_type=2;

		} else {
			g_joystick_type=4;
		} 
		if (g_joystick_type==4) {
         		CheckMenuItem(GetMenu(g_hwnd_main),ID_FILE_JOYSTICK,
            	                      MF_UNCHECKED);
       		} else {
		        CheckMenuItem(GetMenu(g_hwnd_main),ID_FILE_JOYSTICK,
                         	      MF_CHECKED);
       		}
		g_config_kegs_update_needed=1;
       		break;
   	case ID_FILE_DISK:
          	DialogBoxParam(GetModuleHandle(NULL), 
          	               (LPCTSTR)IDD_DLG_DISKCONF,
          	               g_hwnd_main, 
          	               (DLGPROC)win_dialog_disk,0);
    		break;
   	case ID_SPEED_1MHZ:
      		g_limit_speed = 1;
		g_config_kegs_update_needed=1;
      		break;
   	case ID_SPEED_2MHZ:
      		g_limit_speed = 2;
		g_config_kegs_update_needed=1;
      		break;
   	case ID_SPEED_8MHZ:
      		g_limit_speed = 3;
		g_config_kegs_update_needed=1;
      		break;
   	case ID_SPEED_FMHZ:
      		g_limit_speed = 0;
		g_config_kegs_update_needed=1;
      		break;
        default:
                break;
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
	g_quit_sim_now = 1;
	my_exit(0);
}

void
win_event_redraw()
{
	g_full_refresh_needed = -1;
	g_a2_screen_buffer_changed = -1;
	g_status_refresh_needed = 1;
	g_border_sides_refresh_needed = 1;
	g_border_special_refresh_needed = 1;
}

LRESULT CALLBACK
win_event_handler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
	switch(umsg) {
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		win_event_mouse(wParam, lParam);
		return 0;
	case WM_PAINT:
		win_event_redraw();
		break;
	}
	switch(umsg) {
		HANDLE_MSG(hwnd, WM_KEYUP, win_event_key);
		HANDLE_MSG(hwnd, WM_KEYDOWN, win_event_key);
		HANDLE_MSG(hwnd, WM_SYSKEYUP, win_event_key);
		HANDLE_MSG(hwnd, WM_SYSKEYDOWN, win_event_key);
		HANDLE_MSG(hwnd, WM_DESTROY, win_event_quit);
                HANDLE_MSG(hwnd, WM_COMMAND, win_event_command);
	}

#if 0
	switch(umsg) {
	case WM_NCACTIVATE:
	case WM_NCHITTEST:
	case WM_NCMOUSEMOVE:
	case WM_SETCURSOR:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_CONTEXTMENU:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_PAINT:

		break;
	default:
		printf("Got umsg2: %d\n", umsg);
	}
#endif

	return DefWindowProc(hwnd, umsg, wParam, lParam);
}


int
_tmain(int argc, char **argv)
{
	WNDCLASS wndclass;
	RECT	rect;
	int	height;
        int iStatusWidths[] = {60, 100,200,300, -1};

	InitCommonControls();

	wndclass.style = 0;
	wndclass.lpfnWndProc = (WNDPROC)win_event_handler;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(NULL);
        wndclass.hIcon = LoadIcon((HINSTANCE) GetModuleHandle(NULL), 
                        MAKEINTRESOURCE(IDC_KEGS32)); 
	wndclass.hCursor = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
	wndclass.hbrBackground = GetStockObject(BLACK_BRUSH);
        wndclass.lpszMenuName =  MAKEINTRESOURCE(IDC_KEGS32);
	wndclass.lpszClassName = _T("Kegs32");

	// Register the window
	if(!RegisterClass(&wndclass)) {
		printf("Registering window failed\n");
		exit(1);
	}

	height = X_A2_WINDOW_HEIGHT + (MAX_STATUS_LINES * 16) + 32;
	g_main_height = height;

	g_hwnd_main = CreateWindow(_T("KEGS32"), 
        _T("KEGS32 - Apple //gs Emulator"),
		WS_TILED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		X_A2_WINDOW_WIDTH, height,
		NULL, NULL, GetModuleHandle(NULL), NULL);

        if (!g_hwnd_main) {
            printf("Window create failed\n");
            exit(1);
        }

        // Create Toolbar   
        g_win_toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                  WS_CHILD | WS_VISIBLE |  CCS_ADJUSTABLE | TBSTYLE_TOOLTIPS,
                  0, 0, 0, 0,
                  g_hwnd_main,(HMENU)IDC_KEGS32, GetModuleHandle(NULL), NULL);
        SendMessage(g_win_toolbar,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);
        {
            TBADDBITMAP tbab;
            TBBUTTON tbb[6];
            int i,j;
            int idCmd[]={ID_FILE_DISK,-1,
                     ID_SPEED_1MHZ,ID_SPEED_2MHZ,ID_SPEED_8MHZ,ID_SPEED_FMHZ};

            ZeroMemory(tbb, sizeof(tbb));

            for (i=0,j=0;i<sizeof(tbb)/sizeof(TBBUTTON);i++) {
                if (idCmd[i] <0) {
                        tbb[i].fsStyle = TBSTYLE_SEP;
                } else {
                       tbb[i].iBitmap = j;
                       tbb[i].idCommand = idCmd[i];
                       tbb[i].fsState = TBSTATE_ENABLED;
                       tbb[i].fsStyle = TBSTYLE_BUTTON;
                       j++;
                }
            }

            tbab.hInst = GetModuleHandle(NULL);
            tbab.nID = IDC_KEGS32;
            SendMessage(g_win_toolbar, TB_ADDBITMAP,j, (LPARAM)&tbab);
            SendMessage(g_win_toolbar, TB_ADDBUTTONS,i, (LPARAM)&tbb);
            GetWindowRect(g_win_toolbar,&g_win_toolbar_height);
        }

        // Create Status
        g_win_status  = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                          WS_CHILD | WS_VISIBLE , 0, 0, 0, 0,
                          g_hwnd_main, 
                          (HMENU)ID_STATUSBAR, GetModuleHandle(NULL), NULL);
        SendMessage(g_win_status, SB_SETPARTS, 5, (LPARAM)iStatusWidths);
        GetWindowRect(g_win_status,&g_win_status_height);
        SendMessage(g_win_toolbar,TB_AUTOSIZE,0,0);
        SendMessage(g_win_status,WM_SIZE,0,0);

	printf("g_hwnd_main = %p, height = %d\n", g_hwnd_main, height);
	GetWindowRect(g_hwnd_main, &rect);
	printf("...rect is: %ld, %ld, %ld, %ld\n", rect.left, rect.top,
		rect.right, rect.bottom);

	g_main_dc = GetDC(g_hwnd_main);

	SetTextColor(g_main_dc, 0);
	SetBkColor(g_main_dc, 0x0000000);

	g_main_cdc = CreateCompatibleDC(g_main_dc);

	g_screen_depth = 24;
	g_screen_mdepth = 32;


	// Call kegsmain
	return kegsmain(argc, argv);
}


void
check_input_events()
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

	return;
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


void
xdriver_end()
{
	printf("xdriver_end\n");
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
		kimage_ptr->dev_handle = CreateDIBSection(g_main_dc,
				g_bmapinfo_ptr, DIB_RGB_COLORS,
				(VOID **)&(kimage_ptr->data_ptr), NULL, 0);
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
	int	a2code;
	int	i;

	printf("Preparing graphics system\n");

	g_num_a2_keycodes = 0;
	for(i = 0; i < 0x7f; i++) {
		a2code = g_a2_key_to_wsym[i][0];
		if(a2code < 0) {
			g_num_a2_keycodes = i;
		}
	}

	g_screen_depth = 24;
	g_screen_mdepth = 32;

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

	GetWindowRect(g_hwnd_main,&g_main_window_saved_rect);
        init_window(g_hwnd_main,TRUE);
	ShowWindow(g_hwnd_main, SW_SHOWDEFAULT);
	UpdateWindow(g_hwnd_main);

	printf("Done with dev_video_init\n");
	fflush(stdout);

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
	int 	center_x=0;
	int 	center_y=0;
	int toolbar_height;
	RECT 	wrect;
    	TCHAR 	buffer[255]; 

	height = 16;
	margin = 0;

	toolbar_height = 
		(g_win_toolbar_height.bottom-g_win_toolbar_height.top);

	if (g_win_fullscreen_state) {
   		GetWindowRect(g_hwnd_main,&wrect);
		center_x=(wrect.right-wrect.left)-X_A2_WINDOW_WIDTH;
		center_x=center_x/2;
		center_y=(wrect.bottom-wrect.top)-X_A2_WINDOW_HEIGHT;
		if (g_win_status_debug) {
			center_y-=(MAX_STATUS_LINES*16);
		}
		center_y=center_y/2;
		toolbar_height=0;
	}

        if (g_win_status_debug) {
        	oldtextcolor = SetTextColor(g_main_dc, 0xffffff);
        	oldbkcolor = SetBkColor(g_main_dc, 0x0);
        	for(line = 0; line < MAX_STATUS_LINES; line++) {
        		buf = g_status_ptrs[line];
        		if(buf != 0) {
        			len = strlen(buf);
        			TextOut(g_main_dc, 2+center_x, 
					X_A2_WINDOW_HEIGHT +
        				height*line + margin +
					toolbar_height+center_y,
		                        buf, len);
        		}
        	}
	        SetTextColor(g_main_dc, oldtextcolor);
	        SetBkColor(g_main_dc, oldbkcolor);
        }

        if (g_win_status !=NULL) {
                SendMessage(g_win_status, SB_SETTEXT,0,(LPARAM)_T("GSport "));
		_stprintf(buffer,_T("Vol:%d"),g_doc_vol);
	        SendMessage(g_win_status, SB_SETTEXT,1,(LPARAM)buffer);
		buf=g_status_ptrs[0];
        	if (buf != NULL) {
        		buf=strstr(g_status_ptrs[0],"sim MHz:");
			len=strchr(buf+9,' ')-buf;
			buffer[len]=0;
            		if (sizeof(TCHAR) <2) {
                		strncpy((char *)buffer,buf,len);
            		} else {
            		    	mbstowcs((wchar_t *)buffer,buf,len);
            		}
            		SendMessageA(g_win_status, SB_SETTEXT,2,(LPARAM) buffer);
        	} else {
            		SendMessageA(g_win_status,SB_SETTEXT,2,(LPARAM)_T(
				"sim MHz:???"));
        	}
        	buf=g_status_ptrs[0];
        	if (buf != NULL) {
        		buf=strstr(g_status_ptrs[0],"Eff MHz:");
			len=strchr(buf+9,',')-buf;
			buffer[len]=0;
            		if (sizeof(TCHAR) <2) {
                		strncpy((char *)buffer,buf,len);
            		} else {
            		    	mbstowcs((wchar_t *)buffer,buf,len);
            		}
            		SendMessageA(g_win_status, SB_SETTEXT,3,(LPARAM) buffer);
        	} else {
            		SendMessageA(g_win_status,SB_SETTEXT,3,(LPARAM)_T(
				"Eff MHz:???"));
	       	}
        	buf=g_status_ptrs[5];
        	if (buf != NULL) {
        		buf=strstr(g_status_ptrs[5],"fast");
			len=buf-&(g_status_ptrs[5][0]);
			buffer[len]=0;
            		if (sizeof(TCHAR) <2) {
            		    strncpy((char *)buffer,g_status_ptrs[5],len);
            		} else {
                	    mbstowcs((wchar_t *)buffer,g_status_ptrs[5],len);
            		}
            		SendMessageA(g_win_status, SB_SETTEXT,4,(LPARAM) buffer);
        	} else {
           
        	}
        }
}


void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
	int width, int height)
{

	void	*bitm_old;
	POINT	point;
    	RECT 	wrect;	
	int toolbar_height= 0;
	int center_x=0;
	int center_y=0;

	point.x = 0;
	point.y = 0;
	ClientToScreen(g_hwnd_main, &point);
	bitm_old = SelectObject(g_main_cdc, kimage_ptr->dev_handle);



	if (!g_win_fullscreen_state) {
        	toolbar_height=
			(g_win_toolbar_height.bottom-g_win_toolbar_height.top);
	} else {
    		GetWindowRect(g_hwnd_main,&wrect);
		center_x=(wrect.right-wrect.left)-X_A2_WINDOW_WIDTH;
		center_x=center_x/2;
		center_y=(wrect.bottom-wrect.top)-X_A2_WINDOW_HEIGHT;
		if (g_win_status_debug) {
			center_y-=(MAX_STATUS_LINES*16);
		}
		center_y=center_y/2;
	}
		
	BitBlt( g_main_dc, 
		destx+center_x, 
		desty+toolbar_height+center_y,
        	width, height,
		g_main_cdc, srcx, srcy, SRCCOPY);

	SelectObject(g_main_cdc, bitm_old);

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
    	DEVMODE dmScreenSettings;
    	int style;

	if (do_full && !g_win_fullscreen_state) {
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= 800;
		dmScreenSettings.dmPelsHeight	= 600;
		dmScreenSettings.dmBitsPerPel	= 24;	
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|
 	                                  DM_PELSHEIGHT;
			
           	if (ChangeDisplaySettings(&dmScreenSettings, 2) 
			!=DISP_CHANGE_SUCCESSFUL) {
			// If 24-bit palette does not work, try 32-bit            
			dmScreenSettings.dmBitsPerPel	= 32;	
           		if (ChangeDisplaySettings(&dmScreenSettings, 2)) {
				printf (
				"-- Unable to switch to fullscreen mode\n");
	                    	printf (
				"-- No 24-bit or 32-bit mode for fullscreen\n");
                    		dmScreenSettings.dmBitsPerPel=-1;
			}
		}

		if (dmScreenSettings.dmBitsPerPel >0) {
			g_win_fullscreen_state=!g_win_fullscreen_state;
            		GetWindowRect(g_hwnd_main,&g_main_window_saved_rect);
                	ChangeDisplaySettings(&dmScreenSettings, 4); 
			style=GetWindowLong(g_hwnd_main,GWL_STYLE);
		        style &= ~WS_CAPTION;
			SetWindowLong(g_hwnd_main,GWL_STYLE,style);
            		g_main_window_menu_saved=GetMenu(g_hwnd_main);
			SetMenu(g_hwnd_main,NULL);
            		ShowWindow(g_win_status,FALSE);
            		ShowWindow(g_win_toolbar,FALSE);
            		SetWindowPos(g_hwnd_main,HWND_TOPMOST,0,0,
            	             GetSystemMetrics(SM_CXSCREEN),
            	             GetSystemMetrics(SM_CYSCREEN),
            	             SWP_SHOWWINDOW);

            	}
	} else {
		if (g_win_fullscreen_state) {
			ChangeDisplaySettings(NULL,0);
            		style=GetWindowLong(g_hwnd_main,GWL_STYLE);
            		style |= WS_CAPTION;
            		SetWindowLong(g_hwnd_main,GWL_STYLE,style);
            		SetMenu(g_hwnd_main,g_main_window_menu_saved);
            		ShowWindow(g_win_status,TRUE);
            		ShowWindow(g_win_toolbar,TRUE);
			init_window(g_hwnd_main,FALSE);
			g_win_fullscreen_state=!g_win_fullscreen_state;
		}
	}
	return;
}
