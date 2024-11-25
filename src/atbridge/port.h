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

struct packet_t
{
	struct at_addr_t dest;
	struct at_addr_t source;
	byte type;
	size_t size;
	byte* data;
	struct packet_t* next;
};

struct packet_queue_t
{
	struct packet_t* head;
	struct packet_t* tail;
};

void queue_init(struct packet_queue_t* queue);
void queue_shutdown(struct packet_queue_t* queue);

void enqueue(struct packet_queue_t* queue, struct at_addr_t dest, struct at_addr_t source, byte type, size_t size, byte data[]);
void enqueue_packet(struct packet_queue_t* queue, struct packet_t* packet);

struct packet_t* dequeue(struct packet_queue_t* queue);
struct packet_t* queue_peek(struct packet_queue_t* queue);

// Insert the packet at the head of the queue, contrary to normal FIFO operation.
void insert(struct packet_queue_t* queue, struct at_addr_t dest, struct at_addr_t source, byte type, size_t size, byte data[]);
void insert_packet(struct packet_queue_t* queue, struct packet_t* packet);



struct packet_port_t
{
	struct packet_queue_t in;
	struct packet_queue_t out;
};

void port_init(struct packet_port_t* port);
void port_shutdown(struct packet_port_t* port);
