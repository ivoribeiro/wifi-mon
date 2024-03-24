
#include <stdint.h>
#include <stdarg.h>
#include "log-handler.h"

// Function to print debug messages
void debugPrint( const char *message, ...) {
    // Initialize variable argument list
    va_list args;
    va_start(args, message);
    if (DEBUG) {
        vprintf(message, args); // Use vprintf to print formatted message
    }
}
