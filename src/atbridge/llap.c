/*
GSport - an Apple //gs Emulator
Copyright (C) 2013-2014 by Peter Neubauer

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

/** This module implements the LLAP port of the bridge. **/

#include <stdbool.h>
#include "../defc.h"
#include "atbridge.h"
#include "port.h"
#include "llap.h"

typedef enum {
	LLAP_DDP_SHORT = 0x01,
	LLAP_DDP_LONG = 0x02,
	LLAP_ENQ = 0x81,
	LLAP_ACK = 0x82,
	LLAP_RTS = 0x84,
	LLAP_CTS = 0x85
} LLAP_TYPES;

const unsigned int LLAP_PACKET_MAX = 603 /* bytes */;
const unsigned int LLAP_PACKET_MIN = 3 /* bytes */;
const double LLAP_IDG = 400 /* microseconds */;
const double LLAP_IFG = 200 /* microseconds */;
const double GAP_TOLERANCE = 4.0;

static struct packet_port_t llap_port;

typedef enum {
	DIALOG_READY,
	DIALOG_GOT_CTS,
	DIALOG_WAIT_IDG
} DIALOG_STATE;
static DIALOG_STATE dialog_state;
static double dialog_end_dcycs;
static double last_frame_dcycs;

void llap_init()
{
	dialog_state = DIALOG_READY;
	last_frame_dcycs = 0;
	port_init(&llap_port);
}

void llap_shutdown()
{
	port_shutdown(&llap_port);
}


/** Queue one data packet out from the bridge's LLAP port and into the guest. **/
void llap_enqueue_out(struct packet_t* packet)
{
	// Generate the RTS.
	struct packet_t* rts = (struct packet_t*)malloc(sizeof(struct packet_t));
	rts->source.network = packet->source.network;
	rts->source.node = packet->source.node;
	rts->dest.network = packet->dest.network;
	rts->dest.node = packet->dest.node;
	rts->size = 0;
	rts->data = 0;
	rts->type = LLAP_RTS;
	enqueue_packet(&llap_port.out, rts);

	// Enqueue the data.
	enqueue_packet(&llap_port.out, packet);
}

struct packet_t* llap_dequeue_in()
{
	return dequeue(&llap_port.in);
}

static void llap_dump_packet(size_t size, byte data[])
{
	if (size < LLAP_PACKET_MIN)
		atbridge_printf("LLAP short packet.\n");
	else if (size > LLAP_PACKET_MAX)
		atbridge_printf("LLAP long packet.\n");
	else
	{
		at_node_t dest = data[0];
		at_node_t source = data[1];
		LLAP_TYPES type = (LLAP_TYPES)(data[2]);
		
		const char* typeName = 0;
		switch (type)
		{
		case LLAP_DDP_SHORT:
			typeName = "DDP (short)";
			break;
		case LLAP_DDP_LONG:
			typeName = "DDP (long)";
			break;
		case LLAP_ENQ:
			typeName = "lapENQ";
			break;
		case LLAP_ACK:
			typeName = "lapACK";
			break;
		case LLAP_RTS:
			typeName = "lapRTS";
			break;
		case LLAP_CTS:
			typeName = "lapCTS";
			break;
		}

		if (typeName)
			atbridge_printf("LLAP[%d->%d] %s: %d bytes.\n", source, dest, typeName, size);
		else
			atbridge_printf("LLAP[%d->%d] %x: %d bytes.\n", source, dest, type, size);
		
		/*for (size_t i = 0; i < size; i++)
			atbridge_printf("%02x ", data[i]);
		atbridge_printf("\n");*/
	}
}

/** Reply to a control packet from the GS **/
static void llap_reply_control(at_node_t dest, at_node_t source, LLAP_TYPES type)
{
	struct at_addr_t dest_addr = { 0, dest };
	struct at_addr_t source_addr = { 0, source };

	// Insert control packets at the head of the queue contrary to normal FIFO queue operation 
	// to ensure that control frames arrive in the intended order.
	insert(&llap_port.out, dest_addr, source_addr, type, 0, 0);
}

/** Accept a data packet from the GS. **/
static void llap_handle_data(size_t size, byte data[])
{	
	at_node_t dest = data[0];
	at_node_t source = data[1];
	LLAP_TYPES type = (LLAP_TYPES)(data[2]);

	const size_t data_size = size - 3;
	byte* data_copy = (byte*)malloc(data_size);
	memcpy(data_copy, data + 3, data_size);

	struct at_addr_t dest_addr = { 0, dest };
	struct at_addr_t source_addr = { 0, source };
	enqueue(&llap_port.in, dest_addr, source_addr, type, data_size, data_copy);
}

/** Accept a control packet from the GS. **/
static void llap_handle_control(size_t size, byte data[])
{	
	at_node_t dest = data[0];
	at_node_t source = data[1];
	LLAP_TYPES type = (LLAP_TYPES)(data[2]);

	struct at_addr_t addr = { atbridge_get_net(), dest };

	switch (type)
	{
	case LLAP_ENQ:
		// Require the GS to take a valid "client" address not known to be in use.
		if (dest > 127 || dest == 0 || atbridge_address_used(&addr))
			llap_reply_control(source, dest, LLAP_ACK);
		break;
	case LLAP_ACK:
		break;
	case LLAP_RTS:
		if (dest != at_broadcast_node)
			// The GS is trying to make a directed transmission.  Provide the required RTS/CTS handshake.
			// Note that broadcast packets do not require a CTS.
			llap_reply_control(source, dest, LLAP_CTS);
		break;
	case LLAP_CTS:
		// The GS sent a CTS.  If the bridge has pending data, prepare to deliver the packet.
		dialog_state = DIALOG_GOT_CTS;
		break;
	default:
		break;
	}
}

/** Occassionally, we receive an invalid packet from the GS.  I'm unsure if this is due to a bug in GS/OS
    or, more likely, a bug in the SCC emulation.  Regardless, when such a thing does occur, discard the 
	current, corrupted dialog.  Link errors are routine in real LocalTalk networks, and LocalTalk will recover.
	**/
static void llap_reset_dialog()
{
	dialog_state = DIALOG_READY;
	last_frame_dcycs = 0;

	// Discard packets until the queue is either empty or the next dialog starts (and dialogs begin with an RTS).
	while (true)
	{
		struct packet_t* packet = queue_peek(&llap_port.out);
		
		if (packet && (packet->type != LLAP_RTS))
		{
			packet = dequeue(&llap_port.out);
			if (packet->data)
				free(packet->data);
			free(packet);
		}
		else
			break;
	}
}

/** Transfer (send) one LLAP packet from the GS. **/
void llap_enqueue_in(double dcycs, size_t size, byte data[])
{
	atbridge_printf("<%0.0f> TX: ", dcycs);
	llap_dump_packet(size, data);

	if (size < LLAP_PACKET_MIN)
		atbridge_printf("ATBridge: Dropping LLAP short packet.\n");
	else if (size > LLAP_PACKET_MAX)
		atbridge_printf("ATBridge: Dropping LLAP long packet.\n");
	else
	{
		last_frame_dcycs = dcycs;
		LLAP_TYPES type = (LLAP_TYPES)(data[2]);

		switch (type)
		{
		case LLAP_DDP_SHORT:
		case LLAP_DDP_LONG:
			llap_handle_data(size, data);
			break;
		case LLAP_ENQ:
		case LLAP_ACK:
		case LLAP_RTS:
		case LLAP_CTS:
			llap_handle_control(size, data);
			break;
		default:
			// Intentionally check for valid types and ingore packets with invalid types.
			// Sometimes, the bridge gets invalid packets from the GS, which tends to break the bridge.
			atbridge_printf("ATBridge: Dropping LLAP packet with invalid type.\n");
			llap_reset_dialog();
		}
	}
}

/** Transfer (receive) one LLAP packet to the GS. **/
void llap_dequeue_out(double dcycs, size_t* size, byte* data[])
{
	*size = 0;

	// The LocalTalk protocol requires a minimum 400us gap between dialogs (called the IDG).
	// If necessary, wait for the IDG.
	if (dialog_state == DIALOG_WAIT_IDG)
    {
		if ((dcycs - dialog_end_dcycs) >= LLAP_IDG)
			// The IDG is done.
			dialog_state = DIALOG_READY;
		else
			// Continue waiting for the IDG.
			return;
    }
	// The LocalTalk protocols requires a maximum 200us gap between frames within a dialog (called the IFG).
	// If we exceed the IFG, the bridge must be stuck in an incomplete or corrupt dialog.  In this case,
	// discard the current dialog and try again.
	if ((dialog_state != DIALOG_READY) && (last_frame_dcycs != 0) && ((dcycs - last_frame_dcycs) >= (GAP_TOLERANCE*LLAP_IFG)))
	{
		llap_reset_dialog();
		atbridge_printf("ATBridge: Dialog reset due to IFG violation.\n");
	}

	struct packet_t* packet = queue_peek(&llap_port.out);

	if ((dialog_state == DIALOG_READY) && (packet) && !(packet->type & 0x80) && (last_frame_dcycs != 0) && ((dcycs - last_frame_dcycs) >= (GAP_TOLERANCE*LLAP_IDG)))
	{
		llap_reset_dialog();
		packet = queue_peek(&llap_port.out);
		atbridge_printf("ATBridge: Dialog reset due to IDG violation.\n");
	}

	if (packet &&
		((packet->type & 0x80) || /* Pass along control frames without waiting for a CTS. */
		 (!(packet->type & 0x80) && (packet->dest.node == at_broadcast_node) && (dialog_state == DIALOG_READY)) || /* Pass along broadcast frames, which don't wait for CTS frames. */
		 (!(packet->type & 0x80) && (packet->dest.node != at_broadcast_node) && (dialog_state == DIALOG_GOT_CTS)))) /* Pass along directed frames only after receiving a CTS handshake. */
	{
		dequeue(&llap_port.out);

		// Prepend the LLAP header.
		*size = packet->size + 3 + 2;
		*data = (byte*)malloc(*size);
		(*data)[0] = packet->dest.node;
		(*data)[1] = packet->source.node;
		(*data)[2] = packet->type;

		// Insert the data into the new LLAP packet.
		if (*size)
			memcpy((*data) + 3, packet->data, packet->size);

		// Fake a frame check sequence (FCS).  Since our SCC emulation doesn't actually
		// check the FCS, the value of the FCS doesn't matter.
		(*data)[packet->size + 3 + 0] = 0xff;
		(*data)[packet->size + 3 + 1] = 0xff;

		atbridge_printf("<%0.0f> RX: ", dcycs);
		llap_dump_packet(*size, *data);

		if (packet->type & 0x80)
			dialog_state = DIALOG_READY;
		else
		{
			// This was the last packet in the dialog.
			dialog_state = DIALOG_WAIT_IDG;
			dialog_end_dcycs = dcycs;
		}
		
		last_frame_dcycs = dcycs;

		free(packet->data);
		free(packet);
	}
}
