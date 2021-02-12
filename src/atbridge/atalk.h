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

typedef byte at_node_t;
static const at_node_t at_broadcast_node = 0xFF;

typedef word16 at_network_t;

#pragma pack(push, 1)
struct at_addr_t
{
	at_network_t network;
	at_node_t node;
};
#pragma pack(pop)

enum LAP_TYPES { /* reference C-6 */
	LAP_DDP_SHORT = 0x01,
	LAP_DDP_LONG = 0x02
};

enum DDP_SOCKETS { /* reference C-7 */
	DDP_SOCKET_INVALID_00 = 0x00,
	DDP_SOCKET_RTMP = 0x01,
	DDP_SOCKET_NIS = 0x02,
	DDP_SOCKET_ECHO = 0x04,
	DDP_SOCKET_ZIS = 0x06,
	DDP_SOCKET_INVALID_FF = 0xFF,
};

enum DDP_TYPES { /* reference C-6 */
	DDP_TYPE_INVALID = 0x00,
	DDP_TYPE_RTMP = 0x01,
	DDP_TYPE_NBP = 0x02,
	DDP_TYPE_ATP = 0x03,
	DDP_TYPE_AEP = 0x04,
	DDP_TYPE_RTMP_REQUEST = 0x05,
	DDP_TYPE_ZIP = 0x06,
	DDP_TYPE_ADSP = 0x07,
	DDP_TYPE_RESERVED_08 = 0x08,
	DDP_TYPE_RESERVED_09 = 0x09,
	DDP_TYPE_RESERVED_0A = 0x0A,
	DDP_TYPE_RESERVED_0B = 0x0B,
	DDP_TYPE_RESERVED_0C = 0x0C,
	DDP_TYPE_RESERVED_0D = 0x0D,
	DDP_TYPE_RESERVED_0E = 0x0E,
	DDP_TYPE_RESERVED_0F = 0x0F
};

#pragma pack(push, 1)
struct DDP_LONG
{
	byte length[2];
	word16 checksum;
	at_network_t dest_net;
	at_network_t source_net;
	at_node_t dest_node;
	at_node_t source_node;
	byte dest_socket;
	byte source_socket;
	byte type;
};

struct DDP_SHORT
{
	byte length[2];
	byte dest_socket;
	byte source_socket;
	byte type;
};

enum RTMP_FUNCTIONS { /* reference C-8*/
	RTMP_FUNCTION_REQUEST = 0x01,
	RTMP_FUNCTION_RDR_SPLIT = 0x02,
	RTMP_FUNCTION_RDR_NO_SPLIT = 0x03
};

struct rtmp_request_t
{
	byte function;
};

struct rtmp_nonextended_data_t
{
	at_network_t net;
	byte id_length;
	at_node_t node;
	word16 zero;
	byte delimiter;
};

struct rtmp_nonextended_response_t
{
	at_network_t net;
	byte id_length;
	at_node_t node;
};

struct rtmp_extended_data_t
{
	at_network_t net;
	byte id_length;
	at_node_t node;
};

struct rtmp_nonextended_tuple_t
{
	at_network_t net;
	byte distance;
};

struct rtmp_extended_tuple_t
{
	at_network_t range_start;
	byte distance;
	at_network_t range_end;
	byte delimiter;
};

static const byte RTMP_TUPLE_DELIMITER = 0x82;
#pragma pack(pop)
