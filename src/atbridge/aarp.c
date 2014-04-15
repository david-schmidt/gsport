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

/** This module implements AARP, a necessary protocol for ELAP communication. **/

#include <stdbool.h>
#include <time.h>
#include "../defc.h"
#include "atbridge.h"
#include "elap.h"
#include "port.h"
#include "elap_defs.h"
#include "aarp.h"

#ifdef WIN32
#include <winsock.h>
#elif __linux__
#include <netinet/in.h>
#endif

struct amt_entry_t
{
	struct at_addr_t protocol;
	struct ether_addr_t hardware;

	struct amt_entry_t* next;
};

typedef struct amt_entry_t* amt_t;

static amt_t amt = 0;

static unsigned int retry_count;
static clock_t retry_timer;


void aarp_init()
{
	aarp_retry_reset();
}

void aarp_shutdown()
{
	struct amt_entry_t* entry = amt;
	while (entry)
	{
		struct amt_entry_t* next = entry->next;
		free(entry);
		entry = next;
	}
}

////

static void aarp_send_packet(enum AARP_FUNCTION function, const struct at_addr_t* source_at_addr, const struct at_addr_t* dest_at_addr, const struct ether_addr_t* dest_hw_addr)
{
	if (source_at_addr && dest_at_addr && dest_hw_addr)
	{
		struct aarp_header_t response;
		response.hardware_type = htons(AARP_HARDWARE_ETHER);
		response.protocol_type = htons(AARP_PROTOCOL_TYPE);
		response.hw_addr_len = AARP_HW_ADDR_LEN;
		response.protocol_addr_len = AARP_PROTOCOL_ADDR_LEN;
		response.function = htons(function);

		memcpy(&response.source_proto_addr.addr, source_at_addr, sizeof(response.source_proto_addr.addr));
		response.source_proto_addr.addr.network = htons(response.source_proto_addr.addr.network);
		response.source_proto_addr.zero = 0x00;

		memcpy(&response.dest_proto_addr.addr, dest_at_addr, sizeof(response.dest_proto_addr.addr));
		response.dest_proto_addr.addr.network = htons(response.dest_proto_addr.addr.network);
		response.dest_proto_addr.zero = 0x00;

		memcpy(response.source_hw_addr.mac, elap_get_mac()->mac, sizeof(response.source_hw_addr.mac));

		memcpy(response.dest_hw_addr.mac, &dest_hw_addr->mac, sizeof(response.dest_hw_addr.mac));

		if (dest_hw_addr == &HW_ZERO)
			elap_send(&HW_APPLETALK_BROADCAST, &SNAP_AARP, sizeof(struct aarp_header_t), (byte*)&response);
		else
			elap_send(&response.dest_hw_addr, &SNAP_AARP, sizeof(struct aarp_header_t), (byte*)&response);
	}
}

void aarp_probe(const struct at_addr_t* addr)
{
	if (addr)
	{
		aarp_send_packet(AARP_FUNCTION_PROBE, addr, addr, &HW_ZERO);
	}
}

static void aarp_request(const struct at_addr_t* addr)
{
	if (addr)
	{
		aarp_send_packet(AARP_FUNCTION_REQUEST, atbridge_get_addr(), addr, &HW_ZERO);
	}
}

////

static struct amt_entry_t* amt_lookup_entry_hardware(const struct ether_addr_t* hardware)
{
	if (hardware)
	{
		struct amt_entry_t* entry = amt;
		while (entry)
		{
			if (memcmp(&entry->hardware, hardware, sizeof(entry->hardware)) == 0)
				return entry;
			entry = entry->next;
		}
	}
	return 0;
}

static struct amt_entry_t* amt_lookup_entry_protocol(const struct at_addr_t* protocol)
{
	if (protocol)
	{
		struct amt_entry_t* entry = amt;
		while (entry)
		{
			if (memcmp(&entry->protocol, protocol, sizeof(entry->protocol)) == 0)
				return entry;
			entry = entry->next;
		}
	}
	return 0;
}

static void amt_delete_entry_protocol(const struct at_addr_t* protocol)
{
	if (protocol)
	{
		struct amt_entry_t* entry = amt;
		struct amt_entry_t* previous = amt;
		while (entry)
		{
			if (memcmp(&entry->protocol, protocol, sizeof(entry->protocol)) == 0)
			{
				previous->next = entry->next;
				free(entry);
				break;
			}
			previous = entry;
			entry = entry->next;
		}
	}
}

static void amt_add(const struct at_addr_t* protocol, const struct ether_addr_t* hardware)
{
	// Does an entry matching one of the protocol or hardware addresses exist?  If so, update it.
	struct amt_entry_t* entry = amt_lookup_entry_protocol(protocol);
	if (entry)
	{
		memcpy(&entry->hardware, hardware, sizeof(entry->hardware));
		return;
	}

	entry = amt_lookup_entry_hardware(hardware);
	if (entry)
	{
		memcpy(&entry->protocol, protocol, sizeof(entry->protocol));
		return;
	}

	// Otherwise, add a new entry.
	entry = (struct amt_entry_t*)malloc(sizeof(struct amt_entry_t));
	memcpy(&entry->hardware, hardware, sizeof(entry->hardware));
	memcpy(&entry->protocol, protocol, sizeof(entry->protocol));
	entry->next = amt;
	amt = entry;
}

const struct ether_addr_t* aarp_request_hardware(const struct at_addr_t* protocol)
{
	struct amt_entry_t* entry = amt_lookup_entry_protocol(protocol);
	if (entry)
	{
		aarp_retry_reset();
		return (const struct ether_addr_t*)&entry->hardware;
	}
	else
	{
		// The AMT doesn't have this protocol address so issue a request at no more than the AARP_PROBE_INTERVAL period.
		if (((clock() - retry_timer) >= (AARP_REQUEST_INTERVAL * CLOCKS_PER_SEC / 1000)) && 
			(retry_count > 0))
		{
			aarp_request(protocol);
			
			retry_count--;
			retry_timer = clock();
		
			//atbridge_printf("AARP request count %d timer %d.\n", retry_count, retry_timer);
		}

		return 0;
	}
}

const struct at_addr_t* aarp_request_protocol(const struct ether_addr_t* hardware)
{
	struct amt_entry_t* entry = amt_lookup_entry_hardware(hardware);
	if (entry)
		return (const struct at_addr_t*)&entry->protocol;
	else
		return 0;
}

bool aarp_retry()
{
	return retry_count > 0;
}

void aarp_retry_reset()
{
	retry_count = AARP_REQUEST_COUNT;
	retry_timer = clock();
}

void aarp_glean(const struct at_addr_t* protocol, const struct ether_addr_t* hardware)
{
	amt_add(protocol, hardware);
}

bool aarp_address_used(const struct at_addr_t* protocol)
{
	// reference 2-8
	if (protocol)
	{
		// Check for reserved node numbers, per reference 3-9.
		if (protocol->node == 0x00 || protocol->node == 0xfe || protocol->node == 0xff)
			return true;

		// Look for the address in the AMT.  If it's there, another node is using this address.
		struct amt_entry_t* entry = amt_lookup_entry_protocol(protocol);
		if (entry)
			return true;

		// Try a probe.  If this address is in use, another node will reply with an AARP RESPONSE packet.
		// Return true to advise the caller that the address is not known to be in use.  The caller should
		// retry aarp_try_address() every 200 ms (AARP_PROBE_INTERVAL) and 10 times (AARP_PROBE_COUNT),
		// per the AARP protocol definition, before choosing this address.
		aarp_probe(protocol);
		return false;
	}
	return false;
}

////

void aarp_handle_packet(const struct aarp_header_t* aarp)
{
	if (aarp &&
		aarp->hardware_type == AARP_HARDWARE_ETHER &&
		aarp->protocol_type == AARP_PROTOCOL_TYPE &&
		aarp->hw_addr_len == AARP_HW_ADDR_LEN &&
		aarp->protocol_addr_len == AARP_PROTOCOL_ADDR_LEN)
	{
		switch (aarp->function)
		{
		case AARP_FUNCTION_REQUEST:
			if (((aarp->dest_proto_addr.addr.network == atbridge_get_net()) ||
				 (aarp->dest_proto_addr.addr.network == 0x00 /* reference 4-6 */)) &&
				(aarp->dest_proto_addr.addr.node == atbridge_get_node()))
			{
				// Generate a response for the AARP request.
				aarp_send_packet(AARP_FUNCTION_RESPONSE, &aarp->dest_proto_addr.addr, &aarp->source_proto_addr.addr, &aarp->source_hw_addr);
			}
			break;
		case AARP_FUNCTION_RESPONSE:
			aarp_glean(&aarp->source_proto_addr.addr, &aarp->source_hw_addr);
			aarp_glean(&aarp->dest_proto_addr.addr, &aarp->dest_hw_addr);
			break;
		case AARP_FUNCTION_PROBE:
			// AMT entry aging, method 2, reference 2-11
			amt_delete_entry_protocol(&aarp->dest_proto_addr.addr);
			break;
		default:
			break;
		}
	}
}
