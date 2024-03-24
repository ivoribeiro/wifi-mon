#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>// For sleep function
#include <stdint.h>
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
#include <stdarg.h>
#include <signal.h>
#include "client-tracking.h"
#include "log-handler.h"

volatile sig_atomic_t stop_flag = 0;

void signal_handler(int sig) {
    debugPrint("Signal Received: %d \n",sig);
    stop_flag = 1;
}
struct ClientList* clientList;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        debugPrint(stderr, "Usage: %s <interface>\n", argv[0]);
        return 1;
    }

    // Get the interface name from command-line argument
    const char *interface = argv[1];

    int sockfd;
    unsigned char buffer[1500]; // Reduced buffer size
    struct sockaddr saddr;
    int saddr_len = sizeof(saddr);
    clientList = malloc(sizeof(struct ClientList));

    // Create raw socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Initialize client list
    initClientList(clientList);

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
        parse_80211_header(buffer,clientList);
        //printClient(&clientList->head);
    }

    // Close socket
    close(sockfd);
    
    // Print client list and destinations
    printClientList(clientList);
    free(clientList); // Remember to free allocated memory

    return 0;
}


