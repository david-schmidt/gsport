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

#ifdef ACTIVEIPHONE
#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGBitmapContext.h>
#include <CoreFoundation/CFURL.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <Carbon/Carbon.h>
#define ENABLEQD
#endif

#include "stdio.h"
#include "defc.h"
#include "protos_macdriver.h"


word32	g_mac_shift_control_state = 0;
int macUsingCoreGraphics=0;

// Coregraphics context

CGContextRef    offscreenContext = NULL;
char *          bitmapData=NULL;
int             bitmapByteCount;
int             bitmapBytesPerRow;

#ifdef ENABLEQD
WindowRef	g_main_window;
CGrafPtr mac_window_port;
#endif

char *g_clipboard = 0x00;
int   g_clipboard_pos;



extern Kimage g_mainwin_kimage;


int	g_use_shmem = 0;

extern int Verbose;

extern int g_video_act_width;
extern int g_video_act_height;
extern int g_video_act_margin_left;
extern int g_video_act_margin_right;
extern int g_video_act_margin_top;
extern int g_video_act_margin_bottom;
extern int g_screen_depth;
extern int g_force_depth;


int g_screen_mdepth = 0;


extern int g_send_sound_to_file;

//extern int g_quit_sim_now; // OG Not need anymore
extern int g_config_control_panel;


int	g_auto_repeat_on = -1;
int	g_x_shift_control_state = 0;


extern int Max_color_size;

extern word32 g_palette_8to1624[256];
extern word32 g_a2palette_8to1624[256];

int	g_alt_left_up = 1;
int	g_alt_right_up = 1;

extern word32 g_full_refresh_needed;

extern int g_border_sides_refresh_needed;
extern int g_border_special_refresh_needed;
extern int g_status_refresh_needed;

extern int g_lores_colors[];

extern int g_a2vid_palette;

extern int g_installed_full_superhires_colormap;

extern int g_screen_redraw_skip_amt;

extern word32 g_a2_screen_buffer_changed;

int g_upd_count = 0;




void
update_window(void)
{
	
	// OG Not needed
	/*
	SetPortWindowPort(g_main_window);
	PenNormal();
	*/
	
	g_full_refresh_needed = -1;
	g_a2_screen_buffer_changed = -1;
	g_status_refresh_needed = 1;
	g_border_sides_refresh_needed = 1;
	g_border_special_refresh_needed = 1;

	g_upd_count++;
	if(g_upd_count > 250) {
		g_upd_count = 0;
	}

}


void
mac_update_modifiers(word32 state)
{
#ifndef ACTIVEIPHONE

	word32	state_xor;
	int	is_up;

	state = state & (
					 cmdKey | controlKey |
					 shiftKey | alphaLock | optionKey 
				);
	state_xor = g_mac_shift_control_state ^ state;
	is_up = 0;
	if(state_xor & controlKey) {
		is_up = ((state & controlKey) == 0);
		adb_physical_key_update(0x36, is_up);
	}
	if(state_xor & alphaLock) {
		is_up = ((state & alphaLock) == 0);
		adb_physical_key_update(0x39, is_up);
	}
	if(state_xor & shiftKey) {
		is_up = ((state & shiftKey) == 0);
		adb_physical_key_update(0x38, is_up);
	}
	if(state_xor & cmdKey) {
		is_up = ((state & cmdKey) == 0);
		adb_physical_key_update(0x37, is_up);
	}
	if(state_xor & optionKey) {
		is_up = ((state & optionKey) == 0);
		adb_physical_key_update(0x3a, is_up);
	}
#endif

	g_mac_shift_control_state = state;
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
x_get_kimage(Kimage *kimage_ptr)
{
#ifdef ENABLEQD
	PixMapHandle	pixmap_handle;
	GWorldPtr	world;
	Rect		world_rect;
	OSStatus	err;
#endif
	word32	*wptr;
	byte	*ptr;
	int	row_bytes;
	int	width;
	int	height;
	int	depth, mdepth;
	int	size;

	width = kimage_ptr->width_req;
	height = kimage_ptr->height;
	depth = kimage_ptr->depth;
	mdepth = kimage_ptr->mdepth;

	size = 0;
	if(depth == g_screen_depth) 
		{

			if (!macUsingCoreGraphics)
			
			{
#ifdef ENABLEQD
		SetRect(&world_rect, 0, 0, width, height);
		err = NewGWorld( &world, 0, &world_rect, NULL, NULL, 0);
		pixmap_handle = GetGWorldPixMap(world);
		err = LockPixels(pixmap_handle);
		ptr = (byte *)GetPixBaseAddr(pixmap_handle);
		row_bytes = ((*pixmap_handle)->rowBytes & 0x3fff);
		kimage_ptr->width_act = row_bytes / (mdepth >> 3);
		mac_printf("Got depth: %d, bitmap_ptr: %p, width: %d\n", depth,	ptr, kimage_ptr->width_act);
		mac_printf("pixmap->base: %08x, rowbytes: %08x, pixType:%08x\n",(int)(*pixmap_handle)->baseAddr,(*pixmap_handle)->rowBytes,(*pixmap_handle)->pixelType);
		wptr = (word32 *)(*pixmap_handle);
		mac_printf("wptr: %p=%08x %08x %08x %08x %08x %08x %08x %08x\n",wptr,wptr[0], wptr[1], wptr[2], wptr[3],wptr[4], wptr[5], wptr[6], wptr[7]);
		kimage_ptr->dev_handle = pixmap_handle;
		kimage_ptr->data_ptr = ptr;
#endif
		}
			else
			{
			
				
				kimage_ptr->width_act = width ;
			size = height* kimage_ptr->width_act * mdepth >> 3;
			ptr = (byte *)malloc(size);
			
			if(ptr == 0) {
				mac_printf("malloc for data fail, mdepth:%d\n", mdepth);
				exit(2);
			}
			
			kimage_ptr->data_ptr = ptr;
				kimage_ptr->dev_handle = (void *)-1;
			}
		}
	else {

		/* allocate buffers for video.c to draw into */

		
		kimage_ptr->width_act = width ;
		size = height* kimage_ptr->width_act * mdepth >> 3 ;		
		ptr = (byte *)malloc(size);

		if(ptr == 0) {
			mac_printf("malloc for data fail, mdepth:%d\n", mdepth);
			exit(2);
		}

		kimage_ptr->data_ptr = ptr;
		kimage_ptr->dev_handle = (void *)-1;
	}

	mac_printf("kim: %p, dev:%p data: %p, size: %08x\n", kimage_ptr,
		kimage_ptr->dev_handle, kimage_ptr->data_ptr, size);

}


#ifdef ENABLEQD
PixMapHandle	pixmap_backbuffer=NULL;
GWorldPtr	backbuffer=NULL;
#endif

void
dev_video_init()
{
	int	lores_col;
	int	i;

	printf("Preparing graphics system\n");
	
	// OG Create  backbuffer
	if (!macUsingCoreGraphics)
	{

#ifdef ENABLEQD
		Rect r;
	SetRect(&r, 0, 0, 704, 462);
	QDErr err = NewGWorld( &backbuffer, 0, &r, NULL, NULL, 0);
	pixmap_backbuffer = GetGWorldPixMap(backbuffer);
#endif
	}
	else
	{
		
	int pixelsWide = 704;
	int pixelsHigh = 462;
		bitmapBytesPerRow   = (pixelsWide * 4);// 1
		bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
		
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		bitmapData = (char*)malloc( bitmapByteCount );// 3
		if (bitmapData == NULL)
		{
			fprintf (stderr, "Memory not allocated!");
			return ;
		}
		offscreenContext = CGBitmapContextCreate (bitmapData,// 4
										 pixelsWide,
										 pixelsHigh,
										 8,      // bits per component
										 bitmapBytesPerRow,
										 colorSpace,
										 kCGImageAlphaNoneSkipLast);
		if (offscreenContext== NULL)
		{
			free (bitmapData);// 5
			fprintf (stderr, "Context not created!");
			return ;
		}
		
		CGContextSetRGBFillColor (offscreenContext, 1, 0.5, 0.5, 1);
		CGContextFillRect (offscreenContext, CGRectMake (0,0, 704, 462 ));
		
		CGColorSpaceRelease( colorSpace );// 6
	}

	
	video_get_kimages();

	if(g_screen_depth != 8) {
		// Get g_mainwin_kimage
		video_get_kimage(&g_mainwin_kimage, 0, g_screen_depth,
							g_screen_mdepth);
	}

	for(i = 0; i < 256; i++) {
		lores_col = g_lores_colors[i & 0xf];
		video_update_color_raw(i, lores_col);
		g_a2palette_8to1624[i] = g_palette_8to1624[i];
	}

	g_installed_full_superhires_colormap = 1;

	fflush(stdout);

}



void
x_push_kimage(Kimage *kimage_ptr, int destx, int desty, int srcx, int srcy,
		int width, int height)
{
	int i;

	if (!macUsingCoreGraphics)
	{
#ifdef ENABLEQD
	PixMapHandle pixmap_handle;
	Rect	src_rect, dest_rect;
	CGrafPtr window_port;

	pixmap_handle = (PixMapHandle)kimage_ptr->dev_handle;
	SetRect(&src_rect, srcx, srcy, srcx + width, srcy + height);
	SetRect(&dest_rect, destx, desty, destx + width, desty + height);

#ifndef 	ACTIVEGSPLUGIN
	SetPortWindowPort(g_main_window);
	window_port = GetWindowPort(g_main_window);

	CopyBits( (BitMap *)(*pixmap_handle),
		GetPortBitMapForCopyBits(window_port), &src_rect, &dest_rect,
		srcCopy, NULL);
#else // !ACTIVEGSPLUGIN
	// OG Write to the back buffer instead of the display window
	window_port = mac_window_port ;	
	SetGWorld(backbuffer,NULL);

	CopyBits( (BitMap *)(*pixmap_handle),
			GetPortBitMapForCopyBits(backbuffer), &src_rect, &dest_rect,
			 srcCopy, NULL);
#endif // ACTIVEGSPLUGIN
#endif
	}
	else
	{
	
	int wd = kimage_ptr->width_act * kimage_ptr->mdepth>>3;
	int w = width *4;
	 char* ptrdest = bitmapData + bitmapBytesPerRow*desty + destx*4;
	char* srcdest = (char*)kimage_ptr->data_ptr + wd*srcy + srcx*4;
	for(i=0;i<height;i+=1)
	{
		memcpy(ptrdest,srcdest,w);
		ptrdest += bitmapBytesPerRow;
		srcdest += wd;

	}
	}
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
#ifdef ENABLEQD
	if(do_hide) {
		HideCursor();
	} else {
		ShowCursor();
	}
#endif
}


void
update_main_window_size()
{
#ifdef ENABLEQD
	Rect	win_rect;
	int	width, height;
	int	left, excess_height;
	int	top, bottom;
	
	GetPortBounds(GetWindowPort(g_main_window), &win_rect);
	width = win_rect.right - win_rect.left;
	height = win_rect.bottom - win_rect.top;
	g_video_act_width = width;
	g_video_act_height = height;
	
	left = MAX(0, (width - A2_WINDOW_WIDTH) / 2);
	left = MIN(left, BASE_MARGIN_LEFT);
	g_video_act_margin_left = left;
	g_video_act_margin_right = left;
	
	
	excess_height = (height - A2_WINDOW_HEIGHT) / 2;
	bottom = MAX(0, excess_height / 2);		// No less than 0
	bottom = MIN(BASE_MARGIN_BOTTOM, bottom);	// No more than 30
	g_video_act_margin_bottom = bottom;
	excess_height -= bottom;
	top = MAX(0, excess_height);
	top = MIN(BASE_MARGIN_TOP, top);
	g_video_act_margin_top = top;
#endif
}


// OG Adding release
void x_release_kimage(Kimage* kimage_ptr)
{
	if (kimage_ptr->dev_handle == (void*)-1)
	{
		free(kimage_ptr->data_ptr);
		kimage_ptr->data_ptr = NULL;
	}
	else
	{
		if (!macUsingCoreGraphics)
		{
#ifdef ENABLEQD
		UnlockPixels((PixMapHandle)kimage_ptr->dev_handle);
		kimage_ptr->dev_handle = NULL;
		DisposeGWorld((GWorldPtr)kimage_ptr->dev_handle2);
		kimage_ptr->dev_handle2 = NULL;
#endif
		}
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
#define CHUNK_SIZE 1024
	char buffer[CHUNK_SIZE];
	int bufsize = 1;
	void *expanding_buffer = 0x00;
	if (g_clipboard)
	{
		g_clipboard_pos = 0;
		free(g_clipboard);
		g_clipboard = 0x00;
	}
	FILE *pipe = popen("pbpaste", "r");
	if (pipe)
	{
		expanding_buffer = calloc(CHUNK_SIZE+1,1);
		bufsize = CHUNK_SIZE;
		while (!feof(pipe))
		{
			if (fgets(buffer, CHUNK_SIZE, pipe) != NULL)
			{
				while (strlen((char*)expanding_buffer) + strlen(buffer) > bufsize)
				{
					bufsize += CHUNK_SIZE + 1;
					expanding_buffer = realloc(expanding_buffer, bufsize);
				}
				/* Skip the leading return character when this is the first line in the paste buffer */
				if (strlen((char*)expanding_buffer) > 0)
					strcat((char*)expanding_buffer,"\r");
				strncat((char*)expanding_buffer,buffer,strlen(buffer));
				g_clipboard = (char*)expanding_buffer;
			}
		}
	}
}

int clipboard_get_char(void)
{
	if (!g_clipboard)
		return 0;
	if (g_clipboard[g_clipboard_pos] == '\n')
		g_clipboard_pos++;
	if (g_clipboard[g_clipboard_pos] == '\0')
		return 0;
	return g_clipboard[g_clipboard_pos++] | 0x80;
}
