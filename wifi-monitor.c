#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h> // Include for ETH_P_ALL
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#include "client-tracking.h" // Include the client tracking header file
#define MAX_BUF_SIZE 256
// Define IFF_MONITOR if not available
#ifndef IFF_MONITOR
#define IFF_MONITOR 0x1000
#endif

struct ClientList clientList;

void killWirelessProcesses(const char* interface) {
    // Command to list processes using the specified interface
    char cmd[MAX_BUF_SIZE];
    snprintf(cmd, sizeof(cmd), "lsof -i -n -P | grep -i '%s' | awk '{print $2}'", interface);

    // File to store the process IDs
    FILE* fp = popen(cmd, "r");
    if (fp == NULL) {
        perror("Failed to execute command");
        return;
    }

    // Read the output of the command
    char buf[MAX_BUF_SIZE];
    memset(buf, 0, sizeof(buf));
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        // Convert string to integer (process ID)
        int pid = atoi(buf);

        // Kill the process
        if (kill(pid, SIGTERM) == -1) {
            perror("Failed to kill process");
            return;
        } else {
            printf("Process with PID %d using interface %s killed successfully.\n", pid, interface);
        }
    } else {
        printf("No processes using interface %s found.\n", interface);
    }

    // Close the file pointer
    pclose(fp);
}

void set_monitor_mode(const char* interface) {
    char cmd[MAX_BUF_SIZE];

    // Bring down the interface
    snprintf(cmd, sizeof(cmd), "sudo ip link set %s down", interface);
    system(cmd);

    // Set the interface to monitor mode
    snprintf(cmd, sizeof(cmd), "sudo iw dev %s set type monitor", interface);
    system(cmd);

    // Bring the interface back up
    snprintf(cmd, sizeof(cmd), "sudo ip link set %s up", interface);
    system(cmd);

    // Verify the interface mode
    snprintf(cmd, sizeof(cmd), "iw dev %s info", interface);
    system(cmd);
}

// 802.11 header structure (simplified)
struct ieee80211_hdr {
    uint16_t frame_control;
    // Add other fields as needed...
};

// Function to parse 802.11 header and extract SSID from Beacon frames
void parse_80211_header(const uint8_t *buffer) {
    

    // Declare variables for source and destination MAC addresses
    uint8_t source_mac[6];
    uint8_t dest_mac[6];

    // Parse 802.11 header
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)(buffer + sizeof(struct ethhdr));
    
    // Extract frame type and subtype
    uint8_t frame_type = (hdr->frame_control >> 2) & 0x03;
    uint8_t frame_subtype = (hdr->frame_control >> 4) & 0x0F;
    // Declare macAddress variable to hold MAC address
    uint8_t macAddress[6]; 

     // Print frame control fields
    /*     
    printf("Frame Control: %04X\n", hdr->frame_control);
    printf("    Protocol Version: %d\n", (hdr->frame_control & 0x0003));
    printf("    Type: %d\n", frame_type);
    printf("    Subtype: %d\n", frame_subtype);
    printf("    To DS: %d\n", (hdr->frame_control >> 8) & 0x01);
    printf("    From DS: %d\n", (hdr->frame_control >> 9) & 0x01);
    printf("    More Fragments: %d\n", (hdr->frame_control >> 10) & 0x01);
    printf("    Retry: %d\n", (hdr->frame_control >> 11) & 0x01);
    printf("    Power Management: %d\n", (hdr->frame_control >> 12) & 0x01);
    printf("    More Data: %d\n", (hdr->frame_control >> 13) & 0x01);
    printf("    Protected Frame: %d\n", (hdr->frame_control >> 14) & 0x01);
    printf("    Order: %d\n", (hdr->frame_control >> 15) & 0x01); 
    */
    // Extract source MAC address
    memcpy(source_mac, buffer + 6, 6);

    // Extract destination MAC address
    memcpy(dest_mac, buffer, 6);
    if (frame_type == 0) {
        // Handle Management frames
        switch (frame_subtype) {
            case 0x00:
                //printf("Management Frame: Association Request\n");
                updateClientTracking(&clientList, source_mac,dest_mac, true);
                break;
            case 0x01:
               // printf("Management Frame: Association Response\n");
                break;
            case 0x04:
                //printf("Management Frame: Probe Request\n");
                break;
            case 0x05:
                //printf("Management Frame: Probe Response\n");
                break;
            case 0x08: // Beacon frame
                // Extract SSID from Beacon frame
                char ssid[33]; // SSID length is 32 bytes max, plus one for null terminator
                memcpy(ssid, buffer + sizeof(struct ethhdr) + sizeof(struct ieee80211_hdr) + 36, 32);
                ssid[32] = '\0'; // Null-terminate the string
                // Print SSID
                printf("SSID: %s\n", ssid);
                // Extract RSSI from the Beacon frame
                int8_t rssi = buffer[sizeof(struct ethhdr) + sizeof(struct ieee80211_hdr) + 22]; // Example: RSSI at byte offset 22
                printf("RSSI: %d dBm\n", rssi);
                break;
            case 0x0B:
                //printf("Management Frame: Disassociation Request\n");
                // Extract client MAC address from the frame and update client tracking
                updateClientTracking(&clientList,source_mac,dest_mac,false);
                break;
            case 0x0D:
                //printf("Management Frame: Authentication (Open System)\n");
                break;
            case 0x0E:
                //printf("Management Frame: Authentication (Shared Key)\n");
                break;
            // Add more cases for other Management frame subtypes as needed
            default:
                //printf("Unknown Management Frame Subtype: %02X\n", frame_subtype);
                break;
        }
    }
    // Check if it's a Control frame
    else if (frame_type == 1) {
        // Handle Control frames
        switch (frame_subtype) {
            case 0x04:
                //printf("Control Frame: RTS (Request to Send)\n");
            break;
            case 0x08:
                //printf("Control Frame: Block Ack Request\n");
                break;
            case 0x09:
                //printf("Control Frame: Block Ack\n");
                break;
            case 0x0D:
                //printf("Control Frame: PS-Poll (Power Save Poll)\n");
                break;
            // Add more cases for other Control frame subtypes as needed
            default:
                //printf("Unknown Control Frame Subtype: %02X\n", frame_subtype);
                break;
        }
    }
    // Check if it's a Data frame
   else if (frame_type == 2) {
        // Handle Data frames
        switch (frame_subtype) {
            case 0x08:
                //printf("Data Frame: Data\n");
                break;
            case 0x09:
                //printf("Data Frame: Data + CF-ACK\n");
                break;
            case 0x0A:
                //printf("Data Frame: Data + CF-Poll\n");
                break;
            case 0x0B:
               // printf("Data Frame: Null\n");
                break;
            // Add more cases for other Data frame subtypes as needed
            default:
                //printf("Unknown Data Frame Subtype: %02X\n", frame_subtype);
                break;
        }
    }
    // If frame_type is 3, it's reserved and should be ignored
    
    // For other types/subtypes, you can add additional cases or handling as needed
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h> // For sleep function
#include <signal.h> // For signal handling

// Other function declarations
void printClientList(struct ClientList* list);
// Assume other functions are defined elsewhere

volatile sig_atomic_t stop_flag = 0;

void signal_handler(int sig) {
    stop_flag = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    // Get the interface name from command-line argument
    const char *interface = argv[1];

    int sockfd;
    unsigned char buffer[1500]; // Reduced buffer size
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);

    // Create raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Initialize client list
    initClientList(&clientList);

    // Kill processes using the wireless interface
    killWirelessProcesses(interface);

    // Enable promiscuous mode on the wireless interface
    set_monitor_mode(interface);

    // Register signal handler for SIGINT (Ctrl+C) to stop the loop
    signal(SIGINT, signal_handler);

    // Main loop for receiving packets
    while (!stop_flag) {
        // Receive packet
        int packet_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, &saddr, (socklen_t*)&saddr_len);
        if (packet_size < 0) {
            perror("Packet reception failed");
            continue; // Continue listening for packets
        }

        // Parse 802.11 header and extract SSID
        parse_80211_header(buffer);
    }

    // Close socket
    close(sockfd);

    // Print client list and destinations
    printClientList(&clientList);

    return 0;
}


