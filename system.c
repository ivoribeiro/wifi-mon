#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include "global.h" 

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