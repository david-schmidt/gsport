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

/** This module is the "heart" of the bridge and provides the connection between the ELAP and LLAP ports. **/

#include <stdbool.h>
#include "../defc.h"
#include <stddef.h>
#include <time.h>
#include "atbridge.h"
#include "port.h"
#include "elap.h"
#include "llap.h"
#include "aarp.h"

#ifdef WIN32
#include <winsock.h>
#elif __linux__
#include <netinet/in.h>
#endif

extern struct packet_port_t elap_port;

static bool diagnostics = false;
static bool sent_rtmp_request = false;

static struct at_addr_t local_address = { 0, 0 };

static const at_network_t NET_STARTUP_LOW = 0xFF00;
static const at_network_t NET_STARTUP_HIGH = 0xFFFE;
static const at_node_t NODE_STARTUP_LOW = 0x01;
static const at_node_t NODE_STARTUP_HIGH = 0xFE;

static void send_rtmp_request();

bool atbridge_init()
{
	// If the GS reboots, we may try to reinitialize the bridge.  If this is the case, keep the old address and AMT.
	if (local_address.network == 0)
	{
		// Obtain a provisional node address and startup range network.
		//
		// This isn't correct for an extended network (like ELAP) but works adequately on small networks.
		// The bridge should follow the complicated process on page 4-9 to obtain the network and node number.
		srand((unsigned int)time(0));
		local_address.network = (at_network_t)((double)rand()/RAND_MAX * (NET_STARTUP_HIGH - NET_STARTUP_LOW) + NET_STARTUP_LOW);
		local_address.node = (at_node_t)((double)rand()/RAND_MAX + (NODE_STARTUP_HIGH - NODE_STARTUP_LOW) + 0x01);

		aarp_init();
		llap_init();
		if (!elap_init())
		{
			atbridge_shutdown();
			return false;
		}
	}
	return true;
}

void atbridge_shutdown()
{
	llap_shutdown();
	elap_shutdown();
	aarp_shutdown();
}

void atbridge_set_diagnostics(bool enabled)
{
	diagnostics = enabled;
}

bool atbridge_get_diagnostics()
{
	return diagnostics;
}

void atbridge_printf(const char *fmt, ...)
{
	if (atbridge_get_diagnostics())
	{
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

const struct at_addr_t* atbridge_get_addr()
{
	return &local_address;
}

const at_network_t atbridge_get_net()
{
	return local_address.network;
}

const at_node_t atbridge_get_node()
{
	return local_address.node;
}

void atbridge_set_net(at_network_t net)
{
	local_address.network = net;
}

void atbridge_set_node(at_node_t node)
{
	local_address.node = node;
}

bool atbridge_address_used(const struct at_addr_t* addr)
{
	if (!sent_rtmp_request)
		send_rtmp_request();
	return aarp_address_used(addr);
}

/* Calculate a DDP checksum, per Apple's documented algorithm in 4-17 of "Inside AppleTalk". */
static word16 get_checksum(size_t size, byte data[])
{
	word16 cksum = 0;
	for (unsigned int i = 0; i < size; i++)
	{
		cksum += data[i];
		cksum = (cksum << 1) | ((cksum & 0x8000) >> 15); // roll left
	}
	if (cksum == 0)
		cksum = 0xffff;
	return cksum;
}

static void calculate_checksum(struct packet_t* packet)
{
	if (packet && packet->data && (packet->size >= sizeof(struct DDP_LONG)) && (packet->type == LAP_DDP_LONG))
	{
		struct DDP_LONG* header = (struct DDP_LONG*)(packet->data);
		header->checksum = htons(get_checksum(
			packet->size - offsetof(struct DDP_LONG, dest_net),
			(byte*)&header->dest_net));
	}
}

/* Convert a long-form DDP header to a short-form header.  This function only converts the headers. */
static word16 convert_ddp_header_to_short(const struct DDP_LONG* in, struct DDP_SHORT* out)
{
	word16 size;

	if (!in || !out)
		return 0;
	
	size = ((in->length[0] & 0x3) << 8) + (in->length[1]) - (sizeof(struct DDP_LONG) - sizeof(struct DDP_SHORT));

	out->length[0] = (size >> 8) & 0x03;
	out->length[1] = size & 0xff;

	out->dest_socket = in->dest_socket;
	out->source_socket = in->source_socket;
	
	out->type = in->type;

	return size;
}

/* Convert a short-form DDP header to a long-form header.  ELAP requires long-form, but LLAP often uses short-form. */
/* This function only converts the headers. */
static word16 convert_ddp_header_to_long(const struct at_addr_t dest, const struct at_addr_t source, const struct DDP_SHORT* in, struct DDP_LONG* out)
{
	word16 size;

	if (!in || !out)
		return 0;
	
	size = ((in->length[0] & 0x3) << 8) + (in->length[1]) + (sizeof(struct DDP_LONG) - sizeof(struct DDP_SHORT));
	out->length[0] = (size >> 8) & 0x03;
	out->length[1] = size & 0xff;
	
	out->checksum = 0x0000; /* 0x0000 == no checksum calculated, reference 4-17 */
	
	if (dest.network)
		out->dest_net = dest.network;
	else
		out->dest_net = atbridge_get_net();
	out->dest_net = (at_network_t)htons(out->dest_net);

	if (source.network)
		out->source_net = source.network;
	else
		out->source_net = atbridge_get_net();
	out->source_net = (at_network_t)htons(out->source_net);

	out->dest_node = dest.node;
	out->source_node = source.node;

	out->dest_socket = in->dest_socket;
	out->source_socket = in->source_socket;

	out->type = in->type;

	return size;
}

/* Convert a short-form DDP packet to a long-form packet. */
/* This function converts an entire packet, not just the header. */
static void convert_ddp_packet_to_long(struct packet_t* packet)
{
	if (packet && (packet->type == LAP_DDP_SHORT) && packet->data && (packet->size >= sizeof(struct DDP_SHORT)))
	{
		struct DDP_SHORT* header_short = (struct DDP_SHORT*)packet->data;

		const size_t payload_size = packet->size - sizeof(struct DDP_SHORT);
		byte* data = (byte*)malloc(payload_size + sizeof(struct DDP_LONG));
		struct DDP_LONG* header_long = (struct DDP_LONG*)data;

		const word16 size = convert_ddp_header_to_long(packet->dest, packet->source, header_short, header_long);
		packet->dest.network = ntohs(header_long->dest_net);
		packet->source.network = ntohs(header_long->source_net);

		memcpy(data + sizeof(struct DDP_LONG), packet->data + sizeof(struct DDP_SHORT), payload_size);

		packet->type = LAP_DDP_LONG;
		packet->size = size;

		// Replace the original short-form packet data.
		free(packet->data);
		packet->data = data;

		calculate_checksum(packet);
	}
}

/* Convert a long-form DDP packet to short-form. */
static void convert_ddp_packet_to_short(struct packet_t* packet)
{
	if (packet && (packet->type == LAP_DDP_LONG) && packet->data)
	{
		struct DDP_LONG* header_long = (struct DDP_LONG*)packet->data;

		const size_t payload_size = packet->size - sizeof(struct DDP_LONG);
		byte* data = (byte*)malloc(payload_size + sizeof(struct DDP_SHORT));
		struct DDP_SHORT* header_short = (struct DDP_SHORT*)data;

		const word16 size = convert_ddp_header_to_short(header_long, header_short);

		memcpy(data + sizeof(struct DDP_SHORT), packet->data + sizeof(struct DDP_LONG), payload_size);

		packet->type = LAP_DDP_SHORT;
		packet->size = size;

		free(packet->data);
		packet->data = data;
	}
}

/*static void convert_rtmp_to_extended(struct packet_t* packet)
{
	if (packet && (packet->type == LAP_DDP_SHORT) && packet->data)
	{
		struct DDP_SHORT* header_short = (struct DDP_SHORT*)packet->data;
		if (header_short->type != DDP_TYPE_RTMP || header_short->dest_socket != DDP_SOCKET_RTMP)
			return;

		struct rtmp_nonextended_data_t* in = (struct rtmp_nonextended_data_t*)(packet->data + sizeof(struct DDP_SHORT));

		// Construct a new long-form DDP packet header.
		size_t size = sizeof(struct DDP_LONG) + sizeof(struct rtmp_extended_data_t);
		byte* data = (byte*)malloc(size);
		struct DDP_LONG* header_long = (struct DDP_LONG*)data;
		convert_ddp_header_to_long(packet->dest, packet->source, header_short, header_long);
		
		struct rtmp_extended_data_t* out = (struct rtmp_extended_data_t*)(data + sizeof(struct DDP_LONG));
		out->net = in->net;
		out->id_length = in->id_length;
		out->node = in->node;

		// Copy the routing tuples.
		struct rtmp_nonextended_tuple_t* in_tuple = (struct rtmp_nonextended_tuple_t*)(packet->data + sizeof(struct DDP_SHORT) + sizeof(struct rtmp_nonextended_data_t));
		struct rtmp_extended_tuple_t* out_tuple = (struct rtmp_extended_tuple_t*)(data + size);
		while ((byte*)in_tuple < (packet->data + packet->size))
		{
			size += sizeof(struct rtmp_extended_tuple_t);
			realloc(data, size);
			out_tuple->range_start = in_tuple->net;
			out_tuple->distance = in_tuple->distance | 0x80;
			out_tuple->range_end = in_tuple->net;
			out_tuple->delimiter = RTMP_TUPLE_DELIMITER;
			in_tuple++;
		}

		free(packet->data);
		packet->data = data;
		packet->size = size;
		packet->type = LAP_DDP_LONG;
	}
}*/

static void convert_rtmp_to_nonextended(struct packet_t* packet)
{
	if (packet && (packet->type == LAP_DDP_LONG) && packet->data)
	{
		struct DDP_LONG* header_long = (struct DDP_LONG*)packet->data;
		if (header_long->type != DDP_TYPE_RTMP || header_long->dest_socket != DDP_SOCKET_RTMP)
			return;

		struct rtmp_extended_data_t* in = (struct rtmp_extended_data_t*)(packet->data + sizeof(struct DDP_LONG));

		size_t size = sizeof(struct DDP_SHORT) + sizeof(struct rtmp_nonextended_response_t);
		byte* data = (byte*)malloc(size);
		struct DDP_SHORT* header_short = (struct DDP_SHORT*)data;
		convert_ddp_header_to_short(header_long, header_short);
		header_short->length[0] = (size >> 8) & 0x03;
		header_short->length[1] = size & 0xff;

		struct rtmp_nonextended_response_t* out = (struct rtmp_nonextended_response_t*)(data + sizeof(struct DDP_SHORT));
		out->net = in->net;
		out->id_length = in->id_length;
		out->node = in->node;

		/*rtmp_extended_tuple_t* in_tuple = (rtmp_extended_tuple_t*)(packet->data + sizeof(DDP_LONG) + sizeof(rtmp_extended_data_t));
		rtmp_nonextended_tuple_t* out_tuple = (rtmp_nonextended_tuple_t*)(data + size);
		while ((byte*)in_tuple < (packet->data + packet->size))
		{
			size += sizeof(rtmp_nonextended_tuple_t);
			realloc(data, size);
			out_tuple->net = in_tuple->range_start;
			out_tuple->distance = in_tuple->distance & 0x7f;
			in_tuple++;
		}*/

		free(packet->data);
		packet->data = data;
		packet->size = size;
		packet->type = LAP_DDP_SHORT;
	}	
}

/* Learn our network number from RTMP packets. */
/* "Inside AppleTalk", section 4-8, describes this approach for non-extended networks.
   Technically, we probably should be doing the more complicated extended network approach (also on 4-8),
   but the easy approach using RTMP seems adequate for now. */
static void glean_net_from_rtmp(struct packet_t* packet)
{
	if (packet && (packet->type == LAP_DDP_LONG) && packet->data)
	{
		struct DDP_LONG* header_long = (struct DDP_LONG*)packet->data;
		if (header_long->type != DDP_TYPE_RTMP || header_long->dest_socket != DDP_SOCKET_RTMP)
			return;

		struct rtmp_extended_data_t* in = (struct rtmp_extended_data_t*)(packet->data + sizeof(struct DDP_LONG));

		atbridge_set_net(ntohs(in->net));
	}
}

static void send_rtmp_request()
{
	struct packet_t* packet = (struct packet_t*)malloc(sizeof(struct packet_t));
	
	packet->type = LAP_DDP_LONG;
	packet->dest.network = atbridge_get_net();
	packet->dest.node = 255;
	packet->source.network = atbridge_get_net();
	packet->source.node = atbridge_get_node();
	packet->next = 0;
	packet->size = sizeof(struct DDP_LONG) + sizeof(struct rtmp_request_t);
	packet->data = (byte*)malloc(packet->size);

	struct DDP_LONG* header = (struct DDP_LONG*)packet->data;
	header->type = DDP_TYPE_RTMP_REQUEST;
	header->source_net = htons(packet->source.network);
	header->source_node = packet->source.node;
	header->source_socket = DDP_SOCKET_RTMP;
	header->dest_net = htons(packet->dest.network);
	header->dest_node = packet->dest.node;
	header->dest_socket = DDP_SOCKET_RTMP;
	header->length[0] = (packet->size >> 8) & 0x03;
	header->length[1] = packet->size & 0xff;

	struct rtmp_request_t* request = (struct rtmp_request_t*)(packet->data + sizeof(struct DDP_LONG));
	request->function = RTMP_FUNCTION_REQUEST;

	calculate_checksum(packet);
	
	elap_enqueue_out(packet);
	sent_rtmp_request = true;
}

void atbridge_process()
{
	elap_process();
	//llap_process();
	
	struct packet_t* packet = elap_dequeue_in();
	if (packet)
	{
		glean_net_from_rtmp(packet);
		convert_rtmp_to_nonextended(packet);
		// The GS should understand long-form DDP, but converting to short-form ought to slightly improve performance (fewer bytes for the GS to process).
		convert_ddp_packet_to_short(packet);
		llap_enqueue_out(packet);
	}
	packet = llap_dequeue_in();
	if (packet)
	{
		// ELAP does not support short-form DDP, so convert such packets to long-form.
		convert_ddp_packet_to_long(packet);
		elap_enqueue_out(packet);
	}
}