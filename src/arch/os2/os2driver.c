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

#include "../../defc.h"
#include "../../protos.h"

#define INCL_WIN
#define INCL_GPI

#include <os2.h>                        /* PM header file               */
#include <string.h>
#include "gsportos2.h"                  /* Resource symbolic identifiers*/

HAB  g_hab;                               /* PM anchor block handle       */
PSZ  pszErrMsg;
QMSG qmsg;                              /* Message from message queue   */
HWND g_hwnd_frame = NULLHANDLE;            /* Frame window handle          */
HWND g_hwnd_client = NULLHANDLE;         /* Client area window handle    */

HMQ  g_hmq;                               /* Message queue handle         */

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

BITMAPINFO2 *g_bmapinfo_ptr = 0;
volatile BITMAPINFOHEADER2 *g_bmaphdr_ptr = 0;
HDC g_hdc;
HPS g_hps;

int
win_nonblock_read_stdin(int fd, char *bufptr, int len)
{
  return 0;
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
ULONG flCreate;                       /* Window creation control flags*/
int	height;
SIZEL	sizel;

	if ((g_hab = WinInitialize(0)) == 0L)   /* Initialize PM     */
		os2_abort(g_hwnd_frame, g_hwnd_client); /* Terminate the application    */

	if ((g_hmq = WinCreateMsgQueue( g_hab, 0 )) == 0L)/* Create a msg queue */
		os2_abort(g_hwnd_frame, g_hwnd_client); /* Terminate the application    */

	if (!WinRegisterClass(                /* Register window class        */
		g_hab,                               /* Anchor block handle          */
		(PSZ)"MyWindow",                   /* Window class name            */
		(PFNWP)MyWindowProc,               /* Address of window procedure  */
		CS_SIZEREDRAW,                     /* Class style                  */
		0                                  /* No extra window words        */
		))
		os2_abort(g_hwnd_frame, g_hwnd_client); /* Terminate the application    */

	height = X_A2_WINDOW_HEIGHT + (MAX_STATUS_LINES*16) + 32;
	g_main_height = height;

	flCreate = FCF_STANDARD &            /* Set frame control flags to   */
		~FCF_SHELLPOSITION;        /* standard except for shell    */
                                        /* positioning.                 */

	if ((g_hwnd_frame = WinCreateStdWindow(
		HWND_DESKTOP,            /* Desktop window is parent     */
		0,                       /* STD. window styles           */
		&flCreate,               /* Frame control flag           */
		"MyWindow",              /* Client window class name     */
		"",                      /* No window text               */
		0,                       /* No special class style       */
		(HMODULE)0L,             /* Resource is in .EXE file     */
		ID_WINDOW,               /* Frame window identifier      */
		&g_hwnd_client           /* Client window handle         */
		)) == 0L)
	os2_abort(HWND_DESKTOP, HWND_DESKTOP); /* Terminate the application    */

	WinSetWindowText(g_hwnd_frame, "GSport");

	if (!WinSetWindowPos( g_hwnd_frame,      /* Shows and activates frame    */
		HWND_TOP,            /* window at position 100, 100, */
		100, 100, X_A2_WINDOW_WIDTH, height,  /* and size 200, 200.           */
		SWP_SIZE | SWP_MOVE | SWP_ACTIVATE | SWP_SHOW
		))
	os2_abort(g_hwnd_frame, g_hwnd_client); /* Terminate the application    */

	g_hdc = WinOpenWindowDC(g_hwnd_frame);
	sizel.cx = X_A2_WINDOW_WIDTH;
	sizel.cy = height;
	g_hps = GpiCreatePS(g_hab,g_hdc, &sizel, PU_ARBITRARY | GPIT_MICRO | GPIA_ASSOC);

	// Call gsportmain
	return gsportmain(argc, argv);
}


/**************************************************************************
 *
 *  Name       : MyWindowProc
 *
 *  Description: The window procedure associated with the client area in
 *               the standard frame window. It processes all messages
 *               either sent or posted to the client area, depending on
 *               the message command and parameters.
 *
 *************************************************************************/
MRESULT EXPENTRY MyWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  switch( msg )
  {
    case WM_CREATE:
      /*
       * Window initialization is performed here in WM_CREATE processing
       * WinLoadString loads strings from the resource file.
       */
      break;

    case WM_COMMAND:
      /*
       * When the user chooses option 1, 2, or 3 from the Options pull-
       * down, the text string is set to 1, 2, or 3, and
       * WinInvalidateRegion sends a WM_PAINT message.
       * When Exit is chosen, the application posts itself a WM_CLOSE
       * message.
       */
      {
      USHORT command;                   /* WM_COMMAND command value     */
      command = SHORT1FROMMP(mp1);      /* Extract the command value    */
      switch (command)
      {
        case ID_EXITPROG:
          WinPostMsg( hwnd, WM_CLOSE, (MPARAM)0, (MPARAM)0 );
          break;
        default:
          return WinDefWindowProc( hwnd, msg, mp1, mp2 );
      }

      break;
      }
    case WM_ERASEBACKGROUND:
      /*
       * Return TRUE to request PM to paint the window background
       * in SYSCLR_WINDOW.
       */
      return (MRESULT)( TRUE );
    case WM_PAINT:
      /*
       * Window contents are drawn here in WM_PAINT processing.
       */
      {
      HPS    hps;                       /* Presentation Space handle    */
      RECTL  rc;                        /* Rectangle coordinates        */
      POINTL pt;                        /* String screen coordinates    */
                                        /* Create a presentation space  */
      hps = WinBeginPaint( hwnd, 0L, &rc );
      WinFillRect( hps, &rc, SYSCLR_WINDOW);
      pt.x = 50; pt.y = 50;             /* Set the text coordinates,    */
      GpiSetColor( hps, CLR_NEUTRAL );         /* colour of the text,   */
      GpiSetBackColor( hps, CLR_BACKGROUND );  /* its background and    */
      GpiSetBackMix( hps, BM_OVERPAINT );      /* how it mixes,         */
                                               /* and draw the string...*/
      //GpiCharStringAt( hps, &pt, (LONG)strlen( szString ), szString );
      WinEndPaint( hps );                      /* Drawing is complete   */
      break;
      }
    case WM_CLOSE:
      /*
       * This is the place to put your termination routines
       */
      WinPostMsg( hwnd, WM_QUIT, (MPARAM)0,(MPARAM)0 );/* Cause termination*/
      break;
    default:
      /*
       * Everything else comes here.  This call MUST exist
       * in your window procedure.
       */

      return WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  return (MRESULT)FALSE;
} /* End of MyWindowProc */

void
check_input_events()
{

/*
 * Get and dispatch messages from the application message queue
 * until WinGetMsg returns FALSE, indicating a WM_QUIT message.
 */

  /* Need to peek messages, i.e.
  
  	while(PeekMessage(&msg, g_hwnd_main, 0, 0, PM_NOREMOVE)) {
		if(GetMessage(&msg, g_hwnd_main, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			printf("GetMessage returned <= 0\n");
			my_exit(2);
		}
	}
  */

  while( WinGetMsg( g_hab, &qmsg, 0L, 0, 0 ) )
    WinDispatchMsg( g_hab, &qmsg );
  WinDestroyWindow(g_hwnd_frame);           /* Tidy up...                   */
  WinDestroyMsgQueue( g_hmq );             /* Tidy up...                   */
  WinTerminate( g_hab );                   /* Terminate the application    */

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
		g_bmaphdr_ptr->cx = width;
		g_bmaphdr_ptr->cy = -height;

		kimage_ptr->dev_handle = GpiCreateBitmap(
			(HPS)g_hps, (PBITMAPINFOHEADER2)g_bmaphdr_ptr,
			0L, (PBYTE)kimage_ptr->data_ptr,
			(PBITMAPINFO2)g_bmapinfo_ptr);

		size = (width*height*mdepth) >> 3;
		ptr = (byte *)malloc(size);

		if(ptr == 0) {
			printf("malloc for data failed, mdepth: %d\n", mdepth);
			exit(2);
		}

		kimage_ptr->data_ptr = ptr;

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

	return;
}


void
dev_video_init()
{
	printf("Preparing graphics system\n");

	g_screen_depth = 24;
	g_screen_mdepth = 32;

	g_bmapinfo_ptr = (BITMAPINFO2 *)malloc(sizeof(BITMAPINFOHEADER2));
	g_bmaphdr_ptr = (BITMAPINFOHEADER2 *)g_bmapinfo_ptr;
	g_bmaphdr_ptr->cbFix = sizeof(BITMAPINFOHEADER2);
	g_bmaphdr_ptr->cx = A2_WINDOW_WIDTH;
	g_bmaphdr_ptr->cy = -A2_WINDOW_HEIGHT;
	g_bmaphdr_ptr->cPlanes = 1;
	g_bmaphdr_ptr->cBitCount = g_screen_mdepth;
	g_bmaphdr_ptr->ulCompression = BCA_UNCOMP;
	g_bmaphdr_ptr->cclrUsed = 0;

	video_get_kimages();

	if(g_screen_depth != 8) {
		//	Allocate g_mainwin_kimage
		video_get_kimage(&g_mainwin_kimage, 0, g_screen_depth,
						g_screen_mdepth);
	}

	g_installed_full_superhires_colormap = 1;

	printf("Done with dev_video_init\n");
	fflush(stdout);
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
os2_abort(HWND g_hwnd_frame, HWND g_hwnd_client)
{
  exit(-1);
}