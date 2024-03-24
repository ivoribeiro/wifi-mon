#ifndef CLIENT_TRACKING_H
#define CLIENT_TRACKING_H

#include <stdint.h>

// Define a structure to represent a destination
struct Destination {
    uint8_t mac_address[6]; // MAC address of the client
    struct Destination *next; // Pointer to the next destination
};

// Define a structure to represent the list of destinations
struct DestinationList {
    struct Destination *head; // Pointer to the head of the destination list
};

// Define a structure to represent client information
struct Client {
    uint8_t mac_address[6]; // MAC address of the client
    struct DestinationList *destinations; // Pointer to a list of destinations
    struct Client *next; // Pointer to the next client
};

// Define a structure to represent the client map
struct ClientList {
    struct Client *head; // Pointer to the head of the client map
};

// Function declarations
void initDestinationList(struct DestinationList* list);
void printDestinationList(struct DestinationList* list);

void initClientList(struct ClientList* list);
void updateClientTracking(struct ClientList* list, const uint8_t* mac_address,const uint8_t* dest_mac_address, int associated);
void printClient(struct Client* client);
void printClientList(struct ClientList* list);

#endif /* CLIENT_TRACKING_H */
