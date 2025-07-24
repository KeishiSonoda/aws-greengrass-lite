# Design Document: TES Credential Component

## Overview

This document outlines the design for a simple Greengrass Lite component that demonstrates how to use the Token Exchange Service (TES) to obtain AWS credentials. The component is designed for educational purposes and focuses on clarity and simplicity.

## Architecture

The component will be a standalone C program that:

1. Connects to the local TES service
2. Requests AWS credentials
3. Prints the credentials to stdout
4. Exits

The component will use the Greengrass Lite SDK to communicate with the TES service, which is exposed through a local socket interface.

## Components and Interfaces

### Main Component

The component will consist of a single C file with a simple `main()` function that:

1. Initializes the necessary libraries
2. Makes a direct call to the TES service using the local socket interface
3. Processes the response
4. Prints the credentials
5. Handles any errors

### TES Interface

The component will use the local socket interface to communicate with the TES service. Based on the code in the repository, the TES service is available through a Unix domain socket:

- Socket path: `/run/greengrass/aws_iot_tes`

## Data Flow

1. The component starts and initializes the necessary libraries
2. The component establishes a connection to the TES service via the Unix domain socket
3. The component sends a request for credentials
4. The TES service processes the request and returns AWS credentials
5. The component receives the credentials and parses the JSON response
6. The component extracts and prints the credential information
7. The component exits

## Error Handling

The component will implement simple error handling:

1. If the TES service is unavailable, print an error message
2. If the credentials cannot be parsed, print an error message
3. If any other error occurs, print a descriptive error message

## Implementation Details

### Required Libraries

- Standard C libraries for socket communication, string handling, and JSON parsing
- `ggl/json.h` - For parsing JSON responses (if available)

### Code Structure

```c
// Include necessary headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "/run/greengrass/aws_iot_tes"
#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];
    
    // Create socket
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    
    // Connect to TES service
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    
    // Prepare request
    const char *request = "{\"method\":\"request_credentials_formatted\",\"params\":{}}";
    
    // Send request
    if (write(sock, request, strlen(request)) < 0) {
        perror("write");
        close(sock);
        return 1;
    }
    
    // Receive response
    int bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("read");
        close(sock);
        return 1;
    }
    
    buffer[bytes_read] = '\0';
    
    // Parse and print credentials (simplified for example)
    printf("Received credentials:\n%s\n", buffer);
    
    // Clean up
    close(sock);
    
    return 0;
}
```

### Recipe File

The component will include a simple recipe file that defines:

1. Component name and version
2. Dependencies on the TES service
3. Lifecycle rules

## Testing Strategy

Since this is a simple educational component, testing will be minimal:

1. Manual testing to verify that the component can retrieve credentials
2. Verification that the output is readable and contains the expected fields

## Deployment

The component will be deployed as a standard Greengrass component, which requires:

1. Building the C code
2. Creating a recipe file
3. Deploying the component to a Greengrass core device with TES configured