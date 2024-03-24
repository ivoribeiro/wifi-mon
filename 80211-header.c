#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <net/ethernet.h>
#include <stdbool.h>

#include "client-tracking.h"
#include "log-handler.h"


// 802.11 header structure (simplified)
struct ieee80211_hdr {
    uint16_t frame_control;
    // Add other fields as needed...
};

// Function to parse 802.11 header and extract SSID from Beacon frames
void parse_80211_header(const uint8_t *buffer, struct ClientList* clientList) {
    
    // Declare variables for source and destination MAC addresses
    uint8_t source_mac[6];
    uint8_t dest_mac[6];

    // Parse 802.11 header
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(buffer + sizeof(struct ethhdr));
    
    // Extract frame type and subtype
    uint8_t frame_type = (hdr->frame_control >> 2) & 0x03;
    uint8_t frame_subtype = (hdr->frame_control >> 4) & 0x0F;
     // Print frame control fields
    /*     
    debugPrint("Frame Control: %04X\n", hdr->frame_control);
    debugPrint("    Protocol Version: %d\n", (hdr->frame_control & 0x0003));
    debugPrint("    Type: %d\n", frame_type);
    debugPrint("    Subtype: %d\n", frame_subtype);
    debugPrint("    To DS: %d\n", (hdr->frame_control >> 8) & 0x01);
    debugPrint("    From DS: %d\n", (hdr->frame_control >> 9) & 0x01);
    debugPrint("    More Fragments: %d\n", (hdr->frame_control >> 10) & 0x01);
    debugPrint("    Retry: %d\n", (hdr->frame_control >> 11) & 0x01);
    debugPrint("    Power Management: %d\n", (hdr->frame_control >> 12) & 0x01);
    debugPrint("    More Data: %d\n", (hdr->frame_control >> 13) & 0x01);
    debugPrint("    Protected Frame: %d\n", (hdr->frame_control >> 14) & 0x01);
    debugPrint("    Order: %d\n", (hdr->frame_control >> 15) & 0x01); 
    */
    // Extract source MAC address
    memcpy(source_mac, buffer + 6, 6);

    // Extract destination MAC address
    memcpy(dest_mac, buffer, 6);
    if (frame_type == 0) {
        // Handle Management frames
        switch (frame_subtype) {
            case 0x00:
                debugPrint("Management Frame: Association Request\n");
                updateClientTracking(clientList, source_mac,dest_mac, true);
            break;
            case 0x01:
                debugPrint("Management Frame: Association Response\n");
            break;
            case 0x04:
                debugPrint("Management Frame: Probe Request\n");
            break;
            case 0x05:
                debugPrint("Management Frame: Probe Response\n");
            break;
            case 0x06:
                printf("Management Frame: Probe Response\n");
                // Handle Probe Response frame
            break;
            case 0x08: // Beacon frame
                // Extract SSID from Beacon frame
                char ssid[33]; // SSID length is 32 bytes max, plus one for null terminator
                memcpy(ssid, buffer + sizeof(struct ethhdr) + sizeof(struct ieee80211_hdr) + 36, 32);
                ssid[32] = '\0'; // Null-terminate the string
                // Print SSID
                debugPrint("SSID: %s\n", ssid);
                // Extract RSSI from the Beacon frame
                int8_t rssi = buffer[sizeof(struct ethhdr) + sizeof(struct ieee80211_hdr) + 22]; // Example: RSSI at byte offset 22
                debugPrint("RSSI: %d dBm\n", rssi);
            break;
            case 0x0B:
                debugPrint("Management Frame: Disassociation Request\n");
                // Extract client MAC address from the frame and update client tracking
                updateClientTracking(clientList,source_mac,dest_mac,false);
            case 0x0C:
                debugPrint("Management Frame: Probe Response\n");
                // Extract client MAC address from the frame and update client tracking
                updateClientTracking(clientList,source_mac,dest_mac,false);
            break;
            case 0x0D:
                debugPrint("Management Frame: Authentication (Open System)\n");
            break;
            case 0x0E:
                debugPrint("Management Frame: Authentication (Shared Key)\n");
            break;
            // Add more cases for other Management frame subtypes as needed
            default:
                debugPrint("Unknown Management Frame Subtype: %02X\n", frame_subtype);
            break;
        }
    }
    // Check if it's a Control frame
    else if (frame_type == 1) {
        // Handle Control frames
        switch (frame_subtype) {
            case 0x04:
                debugPrint("Control Frame: RTS (Request to Send)\n");
            break;
            case 0x08:
                debugPrint("Control Frame: Block Ack Request\n");
                break;
            case 0x09:
                debugPrint("Control Frame: Block Ack\n");
                break;
            case 0x0A:
                printf("Control Frame: Block Acknowledgment Request\n");
                // Handle Block Acknowledgment Request frame
            case 0x0B:
                printf("Control Frame: Block Acknowledgment Request\n");
                // Handle Block Acknowledgment Request frame
                break;
            case 0x0C:
                printf("Control Frame: Block Acknowledgment Request\n");
                // Handle Block Acknowledgment Request frame
                break;
            case 0x0D:
                debugPrint("Control Frame: PS-Poll (Power Save Poll)\n");
                break;
            case 0x0E:
                debugPrint("Control Frame: Block Acknowledgment\n");
                break;
            // Add more cases for other Control frame subtypes as needed
            default:
                debugPrint("Unknown Control Frame Subtype: %02X\n", frame_subtype);
                break;
        }
    }
    // Check if it's a Data frame
   else if (frame_type == 2) {
        // Handle Data frames
        switch (frame_subtype) {
            case 0x08:
                debugPrint("Data Frame: Data\n");
                break;
            case 0x09:
                debugPrint("Data Frame: Data + CF-ACK\n");
                break;
            case 0x0A:
                debugPrint("Data Frame: Data + CF-Poll\n");
                break;
            case 0x0B:
                debugPrint("Data Frame: Null\n");
                break;
            case 0x0C:
                debugPrint("Data Frame: Null\n");
                break;
            case 0x0D:
                debugPrint("Data Frame: CF-ACK\n");
                break;
            // Add more cases for other Data frame subtypes as needed
            default:
                debugPrint("Unknown Data Frame Subtype: %02X\n", frame_subtype);
                break;
        }
    }
    // If frame_type is 3, it's reserved and should be ignored
    
    // For other types/subtypes, you can add additional cases or handling as needed

}
