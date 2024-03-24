#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "log-handler.h"

// Define a structure to represent client information
struct Client {
    uint8_t mac_address[6]; // MAC address of the client
    struct DestinationList *destinations; // Pointer to a list of destinations
    struct Client* next;
};

// Define a structure to represent the list
struct ClientList {
    struct Client* head;
};

// Define a structure to represent a destination
struct Destination {
    uint8_t mac_address[6]; // MAC address of the destination
    struct Destination *next; // Pointer to the next destination
};

// Define a structure to represent the list of destinations
struct DestinationList {
    struct Destination *head; // Pointer to the head of the destination list
};

// Function to initialize the client list
void initClientList(struct ClientList* list) {
    list->head = NULL;
}

// Function to initialize the destination list
void initDestinationList(struct DestinationList* list) {
    list->head = NULL;
}

// Function to add a client to the list or add a new destination to an existing client
void addClient(struct ClientList* list, const uint8_t* mac_address, const uint8_t* dest_mac_address) {
    struct Client* current = list->head;
    struct Client* prev = NULL;
    // Search for the client in the list
    while (current != NULL) {
        if (memcmp(current->mac_address, mac_address, 6) == 0) {
            debugPrint("Trying to add destination %02X:%02X:%02X:%02X:%02X:%02X to existing client with MAC address %02X:%02X:%02X:%02X:%02X:%02X.\n",
                   dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5],
                   mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
            // Client found, add new destination to the existing client
            addDestination(current->destinations, dest_mac_address);
            return;
        }
        prev = current;
        current = current->next;
    }

    // Client not found, create a new client and add it to the list
    struct Client* newClient = malloc(sizeof(struct Client));

    if (newClient == NULL) {
        // Handle memory allocation error
        return;
    }

    memcpy(newClient->mac_address, mac_address, 6);
    initDestinationList(newClient->destinations);
    addDestination(newClient->destinations, dest_mac_address);

    newClient->next = NULL;
    if (prev == NULL) {
    // Add as the first node in the list
    list->head = newClient;
    } else {
        // Add after the previous node
        prev->next = newClient;
    }
    

    debugPrint("Added new client with MAC address %02X:%02X:%02X:%02X:%02X:%02X and destination %02X:%02X:%02X:%02X:%02X:%02X.\n",
           mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5],
           dest_mac_address[0], dest_mac_address[1], dest_mac_address[2], dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);
}


// Function to remove a client from the list
void removeClient(struct ClientList* list, const uint8_t* client_mac_address, const uint8_t* dest_mac_address) {

    struct Client* current = &list->head;
    struct Client* prev = NULL;
    while (current != NULL) {
        if (memcmp(current->mac_address, client_mac_address, 6) == 0) {
            // Remove the destination from the client's destination list
            removeDestination(current->destinations, dest_mac_address);

            // Now remove the client itself from the list
            if (prev == NULL) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current); // Free the client
            debugPrint("Client with MAC address %02X:%02X:%02X:%02X:%02X:%02X disassociated from destination %02X:%02X:%02X:%02X:%02X:%02X.\n",
                   client_mac_address[0], client_mac_address[1], client_mac_address[2],
                   client_mac_address[3], client_mac_address[4], client_mac_address[5],
                   dest_mac_address[0], dest_mac_address[1], dest_mac_address[2],
                   dest_mac_address[3], dest_mac_address[4], dest_mac_address[5]);
            return;
        }
        prev = current;
        current = current->next;
    }
    debugPrint("Client with MAC address %02X:%02X:%02X:%02X:%02X:%02X not found.\n",
           client_mac_address[0], client_mac_address[1], client_mac_address[2],
           client_mac_address[3], client_mac_address[4], client_mac_address[5]);
}

// Function to add a destination to the list if it doesn't exist already
void addDestination(struct DestinationList* list, const uint8_t* mac_address) {
    // Check if the destination already exists in the list
    struct Destination* current = list->head;
    while (current != NULL) {
        if (memcmp(current->mac_address, mac_address, 6) == 0) {
            // Destination already exists in the list, return without adding
            debugPrint("Destination %02X:%02X:%02X:%02X:%02X:%02X already exists.\n",
                   mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
            return;
        }
        current = current->next;
    }

    // Destination does not exist, add it to the list
    struct Destination* newDestination = malloc(sizeof(struct Destination));
    if (newDestination == NULL) {
        // Handle memory allocation error
        return;
    }
    memcpy(newDestination->mac_address, mac_address, 6);
    newDestination->next = list->head;
    list->head = newDestination;
    debugPrint("Added new destination %02X:%02X:%02X:%02X:%02X:%02X.\n",
           mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
}

// Function to remove a destination from the list
void removeDestination(struct DestinationList* list, const uint8_t* mac_address) {
    struct Destination* current = list->head;
    struct Destination* prev = NULL;

    while (current != NULL) {
        if (memcmp(current->mac_address, mac_address, 6) == 0) {
            if (prev == NULL) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current); // Free the destination
            debugPrint("Destination with MAC address %02X:%02X:%02X:%02X:%02X:%02X removed.\n",
                   mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
            return;
        }
        prev = current;
        current = current->next;
    }
    debugPrint("Destination with MAC address %02X:%02X:%02X:%02X:%02X:%02X not found.\n",
           mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
}


// Function to update client tracking data
void updateClientTracking(struct ClientList* list, const uint8_t* source_mac_address,const uint8_t* dest_mac_address, int associated) {
    if (associated) {
        // Client associated, add to list
        addClient(list, source_mac_address,dest_mac_address);
    } else {
        // Client disassociated, remove from list
        removeClient(list, source_mac_address,dest_mac_address );
    }
}

// Function to print client information
void printClient(struct Client* client) {
    debugPrint("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           client->mac_address[0], client->mac_address[1], client->mac_address[2],
           client->mac_address[3], client->mac_address[4], client->mac_address[5]);
    printDestinationList(client->destinations);
}

// Function to print the client list
void printClientList(struct ClientList* list) {
    struct Client* current = list->head;
    debugPrint("Client %d",current->mac_address);
    debugPrint("Client list:\n");
    while (current != NULL) {
        debugPrint("Client:\n");
        debugPrint("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
               current->mac_address[0], current->mac_address[1], current->mac_address[2],
               current->mac_address[3], current->mac_address[4], current->mac_address[5]);
        debugPrint("  Destinations:\n");
        printDestinationList(current->destinations);
        current = current->next;
    }
}

// Function to print the destination list
void printDestinationList(struct DestinationList* list) {
    struct Destination* current = list->head;
    debugPrint("Destinations:\n");
    while (current != NULL) {
        debugPrint("- %02X:%02X:%02X:%02X:%02X:%02X\n",
               current->mac_address[0], current->mac_address[1], current->mac_address[2],
               current->mac_address[3], current->mac_address[4], current->mac_address[5]);
        current = current->next;
    }
}
   
