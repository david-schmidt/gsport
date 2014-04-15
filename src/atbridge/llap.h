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

struct packet_t;

/** LLAP port of the AppleTalk Bridge **/

void llap_init();
void llap_shutdown();

/** Send one LLAP packet from the GS
 */
void llap_enqueue_in(double dcycs, size_t size, byte data[]);

/** Receive one LLAP packet from the world to the GS and return the size
 */
void llap_dequeue_out(double dcycs, size_t* size, byte* data[]);


void llap_enqueue_out(struct packet_t* packet);
struct packet_t* llap_dequeue_in();
