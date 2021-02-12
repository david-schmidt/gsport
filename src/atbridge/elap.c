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

/** This module implements the ELAP port of the bridge. **/

#include <stdbool.h>
#include "../defc.h"
#include "atbridge.h"
#include "elap.h"
#include "port.h"
#include "aarp.h"
#include "elap_defs.h"
#include "pcap_delay.h"

#ifdef __CYGWIN__
#include <Windows.h>
#include <NspAPI.h>
#endif
#ifdef WIN32
#include <winsock.h>
#include <IPHlpApi.h>
#endif
#ifdef __linux__
#include <netinet/in.h>
#include <netpacket/packet.h>
#endif

extern int g_ethernet_interface;

static pcap_t* pcap_session;

static struct packet_port_t elap_port;
static struct ether_addr_t HW_LOCAL;

/*static void dump_device_list(pcap_if_t* devices)
   {
        int i = 0;
        for(pcap_if_t* device = devices; device; device = device->next)
        {
                printf("%d. %s", ++i, device->name);
                if (device->description)
                        printf(" (%s)\n", device->description);
                else
                        printf(" (No description available)\n");
        }
   }*/

static void elap_clone_host_mac(pcap_if_t* device) {
  if (!device)
    return;

#ifdef WIN32
  ////
  // Extract the device GUID, which Windows uses to identify the device.
  char* name = device->name;
  while (name && *name != '{')
    name++;
  size_t guidLen = strlen(name);

  ////
  // Find and copy the device MAC address.
  ULONG size = sizeof(IP_ADAPTER_ADDRESSES) * 15;
  IP_ADAPTER_ADDRESSES* addresses = malloc(size);

  ULONG result = GetAdaptersAddresses(AF_UNSPEC,
                                      GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, addresses, &size);

  if (result == ERROR_BUFFER_OVERFLOW)
  {
    // The addresses buffer is too small.  Allocate a bigger buffer.
    free(addresses);
    addresses = malloc(size);
    result = GetAdaptersAddresses(AF_UNSPEC,
                                  GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, addresses, &size);
  }

  if (result == NO_ERROR)
  {
    // Search for the desired adapter address.
    IP_ADAPTER_ADDRESSES* current = addresses;
    while (current)
    {
      if (current->PhysicalAddressLength == ETHER_ADDR_LEN && memcmp(current->AdapterName, name, guidLen) == 0)
      {
        memcpy(&HW_LOCAL.mac, &current->PhysicalAddress, sizeof(HW_LOCAL.mac));
        break;
      }
      current = current->Next;
    }
  }
  else
  {
    halt_printf("ATBridge: Failed to find host MAC address (%d).", result);
  }

  free(addresses);
#else
  #ifdef AF_PACKET
  struct pcap_addr* address;
  for (address = device->addresses; address != 0; address = address->next)
    if (address->addr->sa_family == AF_PACKET)
    {
      struct sockaddr_ll* ll = (struct sockaddr_ll*)address->addr;
      memcpy(&HW_LOCAL.mac, ll->sll_addr, sizeof(HW_LOCAL.mac));
    }
  #endif
#endif
}

const struct ether_addr_t* elap_get_mac() {
  return &HW_LOCAL;
}

bool elap_init() {
  port_init(&elap_port);

  memcpy(&HW_LOCAL, &HW_LOCAL_DEFAULT, sizeof(HW_LOCAL));

  pcap_if_t* device;
  pcap_if_t* alldevs;
  int i = 0;

  char errbuf[PCAP_ERRBUF_SIZE];

  // Load the PCAP library.
  if (!pcapdelay_load())
  {
    halt_printf("ATBridge: PCAP not available.\n");
    return false;
  }

  // Retrieve the device list.
  if(pcapdelay_findalldevs(&alldevs, errbuf) == -1)
  {
    atbridge_printf("ATBridge: Error enumerating PCAP devices: %s\n", errbuf);
    return false;
  }
  //dump_device_list(alldevs);

  // Jump to the selected adapter.
  for (device = alldevs, i = 0; i < g_ethernet_interface; device = device->next, i++);
  if (!device)
  {
    halt_printf("ATBridge: PCAP device not found.  Check interface number in settings.\n");
    return false;
  }

  // Clone the MAC address of the underlying interface.  In certain configurations (e.g. Windows with an MS Loopback
  // interface), the interface, even in promiscous mode, filters foreign MAC addresses.
  elap_clone_host_mac(device);

  // Open the adapter,
  if ((pcap_session = pcapdelay_open_live(device->name,         // name of the device
                                          65536,                                // portion of the packet to capture.
                                          // 65536 grants that the whole packet will be captured on all the MACs.
                                          1,                                            // promiscuous mode (nonzero means promiscuous)
                                          1,                                        // read timeout
                                          errbuf                                // error buffer
                                          )) == NULL)
  {
    halt_printf("ATBridge: Unable to open the adapter. Pcap does not support %s.\n", device->name);
    pcapdelay_freealldevs(alldevs);
    return false;
  }

  // The device must support Ethernet because the bridge "speaks" EtherTalk.
  if (pcapdelay_datalink(pcap_session) == DLT_EN10MB)
  {
    pcapdelay_setnonblock(pcap_session, 1, errbuf);

    atbridge_printf("ATBridge: AppleTalk bridging using network device '%s' with Ethernet address %.2X:%.2X:%.2X:%.2X:%.2X:%.2X.\n",
                    device->description, HW_LOCAL.mac[0], HW_LOCAL.mac[1], HW_LOCAL.mac[2], HW_LOCAL.mac[3], HW_LOCAL.mac[4], HW_LOCAL.mac[5]);

    pcapdelay_freealldevs(alldevs);
    return true;
  }
  else
  {
    pcapdelay_close(pcap_session);
    pcap_session = 0;
    halt_printf("ATBridge: Selected network device %s must support Ethernet.\n", device->description);
    pcapdelay_freealldevs(alldevs);
    return false;
  }
}

void elap_shutdown() {
  port_shutdown(&elap_port);
  if (pcap_session)
  {
    pcapdelay_close(pcap_session);
    pcap_session = 0;
  }
  pcapdelay_unload();
}

void elap_enqueue_out(struct packet_t* packet) {
  enqueue_packet(&elap_port.out, packet);
}

struct packet_t* elap_dequeue_in() {
  return dequeue(&elap_port.in);
}

void elap_send(const struct ether_addr_t* dest, const struct snap_discriminator_t* discriminator, size_t size, byte data[]) {
  if (pcap_session && dest && discriminator)
  {
    // Allocate heap space for the frame.
    const size_t frame_size = sizeof(struct ethernet_header_t) + sizeof(struct snap_header_t) + size;
    u_char* frame_data;
    size_t pad = 0;
    if (frame_size < ETHER_MIN_SIZE)
      pad = ETHER_MIN_SIZE - frame_size;
    frame_data = (u_char*)malloc(frame_size + pad);

    // Build the 802.3 header.
    struct ethernet_header_t* ether = (struct ethernet_header_t*)frame_data;
    memcpy(ether->dest.mac, dest, sizeof(ether->dest.mac));
    memcpy(ether->source.mac, HW_LOCAL.mac, sizeof(ether->source.mac));
    ether->length = htons(frame_size - sizeof(struct ethernet_header_t));

    // Build the 802.2 header.
    struct snap_header_t* snap = (struct snap_header_t*)(frame_data + sizeof(struct ethernet_header_t));
    snap->dsap = SNAP_DSAP;
    snap->ssap = SNAP_SSAP;
    snap->control = SNAP_CONTROL;
    memcpy(&snap->discriminator, discriminator, sizeof(snap->discriminator));

    // Add the data payload.
    struct snap_header_t* payload = (struct snap_header_t*)(frame_data + sizeof(struct ethernet_header_t) + sizeof(struct snap_header_t));
    memcpy(payload, data, size);

    // Add padding to meet minimum Ethernet frame size.
    if (pad > 0)
      memset(frame_data + frame_size, 0, pad);

    pcapdelay_sendpacket(pcap_session, frame_data, frame_size + pad);
  }
}

static bool elap_send_one_queued() {
  // Attempt to send one packet out the host network interface.
  struct packet_t* packet = queue_peek(&elap_port.out);
  if (packet)
  {
    // Find the MAC address.
    const struct ether_addr_t* dest;
    if (packet->dest.node == at_broadcast_node)
      dest = &HW_APPLETALK_BROADCAST;
    else
      dest = aarp_request_hardware(&packet->dest);

    // Send it.
    if (dest)
    {
      elap_send(dest, &SNAP_APPLETALK, packet->size, packet->data);

      dequeue(&elap_port.out);
      free(packet->data);
      free(packet);
    }
    else
    {
      // AARP does not have the needed hardware address.  Give AARP time to obtain the address and keep the current packet.
      if (!aarp_retry())
      {
        // However, if AARP has reached the retry limit, the packet is undeliverable.  Discard the packet and move on.
        atbridge_printf("ATBridge: AARP failed to find MAC address for network %d node %d.  Dropping packet.\n", packet->dest.network, packet->dest.node);
        aarp_retry_reset();
        dequeue(&elap_port.out);
        free(packet->data);
        free(packet);
      }
    }
    return true;
  }
  else
    return false;
}

static void elap_send_all_queued() {
  while (elap_send_one_queued());
}

static bool elap_receive_one() {
  if (!pcap_session)
    return false;

  struct pcap_pkthdr header;
  const u_char* packet = pcapdelay_next(pcap_session, &header);
  if (packet)
  {
    // Receive and process one packet from the host network interface.
    ////

    // Check the Ethernet 802.3 header.
    const struct ethernet_header_t* ether = (struct ethernet_header_t*)packet;
    if (header.len > sizeof(struct ethernet_header_t) &&
        ntohs(ether->length) <= ETHER_MAX_SIZE &&
        ntohs(ether->length) > sizeof(struct snap_header_t) &&
        (memcmp(&ether->source, &HW_LOCAL, sizeof(ether->source)) != 0) &&                 /* Ignore packets sent from our node. */
        (memcmp(&ether->dest, &HW_LOCAL, sizeof(ether->dest)) == 0 ||                 /* Accept packets destined for our node ... */
         memcmp(&ether->dest, &HW_APPLETALK_BROADCAST, sizeof(ether->dest)) == 0 /* ... or for broadcast. */)
        )
    {
      // Check the 802.2 SNAP header.
      const struct snap_header_t* snap = (struct snap_header_t*)(packet + sizeof(struct ethernet_header_t));
      if (snap->dsap == SNAP_DSAP &&
          snap->ssap == SNAP_SSAP &&
          snap->control == SNAP_CONTROL)
      {
        if (memcmp(&snap->discriminator, &SNAP_APPLETALK, sizeof(snap->discriminator)) == 0)
        {
          // Handle an AppleTalk packet.
          const size_t payload_size = ntohs(ether->length) - sizeof(struct snap_header_t);
          const u_char* payload = packet + sizeof(struct ethernet_header_t) + sizeof(struct snap_header_t);

          byte* copy = (byte*)malloc(payload_size);
          memcpy(copy, payload, payload_size);

          // ELAP does not support short-form DDP, so this must be a long-form DDP packet.
          struct at_addr_t source, dest;
          if (payload_size >= sizeof(struct DDP_LONG))
          {
            // Extract the protocol address from the header.
            //
            // ELAP really shouldn't be looking at the header for the next level of the protocol stack,
            // but this is a convenient place to glean addresses for the AMT, a process that needs both
            // hardware and protocol addresses.
            struct DDP_LONG* header = (struct DDP_LONG*)copy;
            dest.network = (at_network_t)ntohs(header->dest_net);
            source.network = (at_network_t)ntohs(header->source_net);
            dest.node = header->dest_node;
            source.node = header->source_node;

            enqueue(&elap_port.in, dest, source, LAP_DDP_LONG, payload_size, copy);

            aarp_glean(&source, &ether->source);
          }
          else
            atbridge_printf("ATBridge: Dropping invalid short ELAP frame.\n");
        }
        else if (memcmp(&snap->discriminator, &SNAP_AARP, sizeof(snap->discriminator)) == 0)
        {
          // Handle an AARP packet.
          struct aarp_header_t* aarp = (struct aarp_header_t*)(packet + sizeof(struct ethernet_header_t) + sizeof(struct snap_header_t));
          aarp->dest_proto_addr.addr.network = ntohs(aarp->dest_proto_addr.addr.network);
          aarp->source_proto_addr.addr.network = ntohs(aarp->source_proto_addr.addr.network);
          aarp->function = ntohs(aarp->function);
          aarp->hardware_type = ntohs(aarp->hardware_type);
          aarp->protocol_type = ntohs(aarp->protocol_type);
          aarp_handle_packet(aarp);
        }
      }
    }

    return true;
  }
  else
    return false;
}

static void elap_receive_all() {
  while (elap_receive_one());
}

void elap_process() {
  elap_receive_all();
  elap_send_all_queued();
}
