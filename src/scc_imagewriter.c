/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2013 - 2014 by GSport contributors
 Originally authored by Christopher Mason

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

/* This is an interface between the SCC emulation and the Virtual Imagewriter. */

#include "defc.h"
#include "scc.h"
#include "imagewriter.h"
extern Scc scc_stat[2];
extern word32 g_vbl_count;

extern int g_imagewriter;
extern int g_imagewriter_dpi;
extern char* g_imagewriter_output;
extern int g_imagewriter_multipage;
extern int g_imagewriter_timeout;

word32 imagewriter_vbl_count = 0;
int imagewriter_port_block = 0;
int iw_scc_write = 0;

int scc_imagewriter_init(int port)
{
	Scc	*scc_ptr;
	scc_ptr = &(scc_stat[port]);
	imagewriter_init(g_imagewriter_dpi,612,792,g_imagewriter_output,g_imagewriter_multipage != 0, port);
	scc_ptr->state = 4;
	return 4;
}

/** Transfer data from Imagewriter to the SCC **/
void scc_imagewriter_fill_readbuf(int port, int space_left, double dcycs)
{
	if (iw_scc_write)
	{
	size_t	bytes_read;
	size_t	i;
	byte data[9];
	if (g_imagewriter == 1)
	{
	//Imagewriter LQ self ID string. Tell machine we have a color ribbon and sheet feeder installed.
	data[0] = 0;  //Start bit
	data[1] ='L'; //Printer type is Imagewriter LQ
	data[2] ='Q'; //(cont)
	data[3] ='1'; //15 inch carriage width
	data[4] ='C'; //Color ribbon installed
	data[5] ='F'; //Sheet feeder installed, no envelope attachment
	data[6] ='1'; //Number of sheet feeder bins
	data[7] = 0x0D; //CR terminates string
	data[8] = 0;  //Stop bit
	bytes_read = 9;
	}
	else
	{
	//Imagewriter II self ID string. Tell machine we have a color ribbon and sheet feeder installed.
	data[0] = 0;  //Start bit
	data[1] ='I'; //Printer type is Imagewriter II
	data[2] ='W'; //(cont)
	data[3] ='1'; //10 inch carriage width
	data[4] ='0'; //(cont)
	data[5] ='C'; //Color ribbon installed
	data[6] ='F'; //Sheet feeder installed
	data[7] = 0;  //Stop bit
	bytes_read = 8;
	}
	for(i = 0; i < bytes_read; i++) {
		scc_add_to_readbuf(port, data[i], dcycs);
	}
	iw_scc_write = 0;
	}
}

/** Transfer data from the SCC to the Imagewriter. **/
void scc_imagewriter_empty_writebuf(int port, double dcycs)
{
	Scc* scc_ptr;

	int	rdptr;
	int	wrptr;
	int	len;
	int	done;
	//int	ret;
	unsigned long	bytes_written;

	scc_ptr = &(scc_stat[port]);
	done = 0;
	while(!done) {
		rdptr = scc_ptr->out_rdptr;
		wrptr = scc_ptr->out_wrptr;
		if(rdptr == wrptr) {
			//printf("...rdptr == wrptr\n");
			done = 1;
			break;
		}
		len = wrptr - rdptr;
		if(len < 0) {
			len = SCC_OUTBUF_SIZE - rdptr;
		}
		if(len > 32) {
			len = 32;
		}
		if(len <= 0) {
			done = 1;
			break;
		}
		bytes_written = 1;
		imagewriter_port_block = 1;
		imagewriter_loop(scc_ptr->out_buf[rdptr],port);
		imagewriter_vbl_count = g_vbl_count+(g_imagewriter_timeout*60);
		imagewriter_port_block = 0;
		//printf("Write Imagewriter ret: %d, bytes_written:%d, len:%d\n", ret,
			//(int)bytes_written, len);
		
		if((bytes_written == 0)) {
			done = 1;
			break;
		} else {
			rdptr = rdptr + bytes_written;
			if(rdptr >= SCC_OUTBUF_SIZE) {
				rdptr = rdptr - SCC_OUTBUF_SIZE;
			}
			scc_ptr->out_rdptr = rdptr;
		}
	}
}

//This function handles the automatic timeout of the virtual printer if an
//application doesn't send a form feed at the end of the page. It also
//allows multipage mode Postscript and native printer documents to
//print somewhat how a regular application would.
void imagewriter_update()
{
	if (imagewriter_port_block != 1 && imagewriter_vbl_count != 0 && g_vbl_count >= imagewriter_vbl_count)
	{
		printf("Calling imagewriter_update and flushing!\n");
		imagewriter_feed();
		imagewriter_vbl_count = 0;
	}
	return;
}