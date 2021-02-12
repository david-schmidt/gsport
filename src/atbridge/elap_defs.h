/*
 GSport - an Apple //gs Emulator
 Copyright (C) 2010 - 2019 by GSport contributors
 Copyright (C) 2016 - 2018  Dagen Brock
 
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

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN 6

static const word16 ETHER_MAX_SIZE = 0x5DC;
static const word16 ETHER_MIN_SIZE = 60;
static const byte SNAP_DSAP = 0xAA;
static const byte SNAP_SSAP = 0xAA;
static const byte SNAP_CONTROL = 0x03;

#define OUI_APPLETALK_1 0x08
#define OUI_APPLETALK_2 0x00
#define OUI_APPLETALK_3 0x07
#define TYPE_APPLETALK_1 0x80
#define TYPE_APPLETALK_2 0x9B

#define OUI_AARP_1 0x00
#define OUI_AARP_2 0x00
#define OUI_AARP_3 0x00
#define TYPE_AARP_1 0x80
#define TYPE_AARP_2 0xF3

static const byte AARP_HARDWARE_ETHER = 0x01;
static const word16 AARP_PROTOCOL_TYPE = 0x809B;
static const byte AARP_HW_ADDR_LEN = 6;
static const byte AARP_PROTOCOL_ADDR_LEN = 4;
enum AARP_FUNCTION
{
	AARP_FUNCTION_REQUEST = 0x01,
	AARP_FUNCTION_RESPONSE = 0x02,
	AARP_FUNCTION_PROBE = 0x03
};

// reference C-4
static const long AARP_PROBE_INTERVAL = 200; /* milliseconds */
static const unsigned int AARP_PROBE_COUNT = 10;

// reference 2-9 and 3-9, optional and at developer discretion
static const long AARP_REQUEST_INTERVAL = 200; /* milliseconds */
static const unsigned int AARP_REQUEST_COUNT = 10;

#pragma pack(push, 1)
struct ether_addr_t
{
	byte mac[ETHER_ADDR_LEN];
};

/* Ethernet 802.2/802.3/SNAP header */
struct ethernet_header_t
{
	// 802.3 header
	struct ether_addr_t dest;
	struct ether_addr_t source;
	word16 length;
};

struct snap_discriminator_t
{
	byte oui[3];
	byte type[2];
};

struct snap_header_t
{
	// 802.2 header
	byte dsap;
	byte ssap;
	byte control;

	// SNAP header
	struct snap_discriminator_t discriminator;
};

struct protocol_addr_t
{
	byte zero;  // Reference C-4 and 3-11: The protocol address is four bytes with the high byte zero.
	struct at_addr_t addr;
};

struct aarp_header_t
{
	word16 hardware_type;
	word16 protocol_type;
	byte hw_addr_len;
	byte protocol_addr_len;
	word16 function;
	struct ether_addr_t source_hw_addr;
	struct protocol_addr_t source_proto_addr;
	struct ether_addr_t dest_hw_addr;
	struct protocol_addr_t dest_proto_addr;
};
#pragma pack(pop)

static const struct ether_addr_t HW_APPLETALK_BROADCAST = {{ 0x09, 0x00, 0x07, 0xff, 0xff, 0xff }};
static const struct ether_addr_t HW_LOCAL_DEFAULT = {{0x02 /* unicast, locally administered */, 'A', '2', 'G', 'S', 0x00}};
//static const struct ether_addr_t HW_LOCAL = {{ 0x02 /* unicast, locally administered */, 0x00, 0x4c, 0x4f, 0x4f, 0x50 }};
static const struct ether_addr_t HW_ZERO = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static const struct snap_discriminator_t SNAP_APPLETALK = {{ OUI_APPLETALK_1, OUI_APPLETALK_2, OUI_APPLETALK_3}, {TYPE_APPLETALK_1, TYPE_APPLETALK_2 }};
static const struct snap_discriminator_t SNAP_AARP = {{ OUI_AARP_1, OUI_AARP_2, OUI_AARP_3}, {TYPE_AARP_1, TYPE_AARP_2 }};
