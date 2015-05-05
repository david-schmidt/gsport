/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2013 - 2014 by GSport contributors
 Originally authored by Peter Neubauer

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

/* This is an interface between the SCC emulation and the LAP bridge. */

#include <stdbool.h>
#include "defc.h"
#include "scc.h"

int g_appletalk_bridging;
int g_appletalk_turbo = 0;
int g_appletalk_diagnostics = 0;
int g_appletalk_network_hint = 0;

#ifdef HAVE_ATBRIDGE
#include "atbridge/atbridge.h"
#include "atbridge/llap.h"

extern Scc scc_stat[2];

extern int g_config_gsport_update_needed;
static bool bridge_initialized = false;

void scc_llap_init()
{
	atbridge_set_diagnostics(g_appletalk_diagnostics);
	bridge_initialized = atbridge_init();
	atbridge_set_net(g_appletalk_network_hint);
}

void scc_llap_set_node(byte val)
{
	atbridge_set_node(val);
}

void scc_llap_update()
{
	if (bridge_initialized)
	{
		atbridge_process();

		// Save the AppleTalk network number.  Since the network number does not
		// change very often, this will slightly improve startup time.
		if (g_appletalk_network_hint != atbridge_get_net())
		{
			g_appletalk_network_hint = atbridge_get_net();
			g_config_gsport_update_needed = 1;
		}
	}
}

/** Transfer one packet from the bridge to the SCC **/
void scc_llap_fill_readbuf(int port, int space_left, double dcycs)
{
	atbridge_set_diagnostics(g_appletalk_diagnostics);

	byte* data;

	if (!bridge_initialized)
		return;

	data = NULL;
	size_t	bytes_read;
	size_t	i;

	llap_dequeue_out(dcycs, &bytes_read, &data);

	for(i = 0; i < bytes_read; i++) {
		scc_add_to_readbuf(port, data[i], dcycs);
	}

	free(data);

	// Normally, the bridge updates from the 60 Hz loop, but that alone bottlenecks network throughput.
	scc_llap_update();
}

/** Transfer one packet from the SCC to the AppleTalk bridge. **/
void scc_llap_empty_writebuf(int port, double dcycs)
{
	atbridge_set_diagnostics(g_appletalk_diagnostics);

	Scc* scc_ptr;

	if (!bridge_initialized)
		return;

	int	rdptr;
	int	wrptr;
	int	len;

	scc_ptr = &(scc_stat[port]);

	// If there's data in the output buffer, send it.
	rdptr = scc_ptr->out_rdptr;
	wrptr = scc_ptr->out_wrptr;
	if(rdptr == wrptr)
		return;
		
	len = wrptr - rdptr;
	if (len < 0)
	{
		// The data is not contiguous since it wraps around the end of the buffer.
		// But, this should never happen since this function always empties the entire
		// buffer, and the buffer is large enough to hold the maximum packet size.
		halt_printf("SCC LLAP: Unexpected non-contiguous data.  Dropping packet.\n");
	}
	else
	{
		// The data is contiguous, so read the data directly from the buffer.
		llap_enqueue_in(dcycs, len, &scc_ptr->out_buf[rdptr]);
	}
		
	// Remove the sent data from the output buffer.  Since the buffer contains
	// one complete packet, always send all of the data.
	scc_ptr->out_rdptr = 0;
	scc_ptr->out_wrptr = 0;

	// Latch EOM to indicate that the SCC has sent the message.
	// The timing will be a bit off from the real hardware since we're not 
	// emulating the sending hardware timing and CRC generation.
	scc_ptr->eom = 1;

	// Normally, the bridge updates from the 60 Hz loop, but that alone bottlenecks network throughput.
	scc_llap_update();
}

#else
void scc_llap_init()
{
}

void scc_llap_set_node(byte val)
{
}

void scc_llap_update()
{
}

void scc_llap_fill_readbuf(int port, int space_left, double dcycs)
{
}

void scc_llap_empty_writebuf(int port, double dcycs)
{
}
#endif