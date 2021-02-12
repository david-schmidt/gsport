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

struct at_addr_t;
struct aarp_header_t;
struct ether_addr_t;

void aarp_init();
void aarp_shutdown();

const struct ether_addr_t* aarp_request_hardware(const struct at_addr_t* protocol);
const struct at_addr_t* aarp_request_protocol(const struct ether_addr_t* hardware);

bool aarp_retry();
void aarp_retry_reset();

void aarp_glean(const struct at_addr_t* protocol, const struct ether_addr_t* hardware);

bool aarp_address_used(const struct at_addr_t* protocol);

void aarp_handle_packet(const struct aarp_header_t* aarp);
