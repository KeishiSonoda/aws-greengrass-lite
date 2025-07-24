/**
 * @file tes-credential-component.c
 * @brief A simple component that demonstrates how to use the Token Exchange Service (TES)
 *        to obtain AWS credentials.
 * 
 * This component connects to the local TES service, requests AWS credentials,
 * and prints them to stdout in a readable format. It serves as an educational
 * example for Greengrass component developers who want to learn how to use
 * the TES service in their own components.
 * 
 * The component performs the following steps:
 * 1. Creates a Unix domain socket
 * 2. Connects to the TES service at /run/greengrass/aws_iot_tes
 * 3. Sends a credential request using the request_credentials_formatted method
 * 4. Receives and parses the JSON response
 * 5. Extracts and displays the AWS credentials (Access Key, Secret Key, Session Token)
 * 6. Shows how to format the credentials as environment variables
 * 
 * This is a minimal implementation focused solely on demonstrating TES functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/**
 * Path to the TES service Unix domain socket
 */
#define SOCKET_PATH "/run/greengrass/aws_iot_tes"

/**
 * Buffer size for reading responses from the TES service
 */
#define BUFFER_SIZE 4096

/**
 * @brief Extract a string value from a JSON response
 * 
 * This is a simple JSON parser that extracts a string value for a given key.
 * It's designed specifically for the TES response format and is not a general-purpose JSON parser.
 * 
 * @param json The JSON string to parse
 * @param key The key to search for
 * @param value Buffer to store the extracted value
 * @param value_size Size of the value buffer
 * @return 0 on success, -1 if key not found or error
 */
int extract_json_string(const char *json, const char *key, char *value, size_t value_size) {
    char search_key[256];
    const char *ptr;
    
    // Format the key with quotes for searching
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    // Find the key in the JSON string
    ptr = strstr(json, search_key);
    if (ptr == NULL) {
        return -1; // Key not found
    }
    
    // Move past the key and colon
    ptr = strchr(ptr + strlen(search_key), ':');
    if (ptr == NULL) {
        return -1; // Invalid JSON format
    }
    
    // Skip whitespace
    ptr++;
    while (*ptr && isspace(*ptr)) {
        ptr++;
    }
    
    // Check if it's a string value (starts with quote)
    if (*ptr != '"') {
        return -1; // Not a string value
    }
    
    // Move past the opening quote
    ptr++;
    
    // Copy the value until the closing quote
    size_t i = 0;
    while (*ptr && *ptr != '"' && i < value_size - 1) {
        // Handle escaped characters
        if (*ptr == '\\' && *(ptr + 1)) {
            ptr++;
            if (*ptr == 'n') {
                value[i++] = '\n';
            } else if (*ptr == 't') {
                value[i++] = '\t';
            } else {
                value[i++] = *ptr;
            }
        } else {
            value[i++] = *ptr;
        }
        ptr++;
    }
    
    value[i] = '\0';
    return 0;
}

/**
 * @brief Main function for the TES credential component
 * 
 * This function:
 * 1. Creates a socket to communicate with the TES service
 * 2. Connects to the TES service
 * 3. Requests AWS credentials
 * 4. Prints the credentials to stdout
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 on success, non-zero on failure
 */
int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_un addr;
    
    printf("TES Credential Component - Starting\n");
    
    // Create socket for TES service communication
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
        return 1;
    }
    
    printf("Socket created successfully\n");
    
    // Set up the address structure for the Unix domain socket
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    // Connect to the TES service
    printf("Connecting to TES service at %s\n", SOCKET_PATH);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Error connecting to TES service: %s\n", strerror(errno));
        close(sock);
        return 1;
    }
    
    printf("Connected to TES service successfully\n");
    
    // Prepare the request for credentials
    // Using the request_credentials_formatted method which returns formatted credentials
    const char *request = "{\"method\":\"request_credentials_formatted\",\"params\":{}}";
    
    // Send the request to the TES service
    printf("Sending credential request to TES service\n");
    if (write(sock, request, strlen(request)) < 0) {
        fprintf(stderr, "Error sending request to TES service: %s\n", strerror(errno));
        close(sock);
        return 1;
    }
    
    // Prepare buffer for the response
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    // Receive the response from the TES service
    printf("Waiting for response from TES service\n");
    int bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        fprintf(stderr, "Error reading response from TES service: %s\n", strerror(errno));
        close(sock);
        return 1;
    }
    
    // Ensure the buffer is null-terminated
    buffer[bytes_read] = '\0';
    
    printf("Received %d bytes from TES service\n", bytes_read);
    
    // Clean up socket
    close(sock);
    printf("Connection closed\n");
    
    // Parse and display the credentials
    printf("\n=== AWS Credentials from TES Service ===\n\n");
    
    // Extract credential information
    char access_key[256] = {0};
    char secret_key[256] = {0};
    char session_token[2048] = {0};
    char expiration[64] = {0};
    
    // Extract the access key
    if (extract_json_string(buffer, "AccessKeyId", access_key, sizeof(access_key)) == 0) {
        printf("Access Key ID:     %s\n", access_key);
    } else {
        fprintf(stderr, "Error: Could not extract AccessKeyId from response\n");
    }
    
    // Extract the secret key
    if (extract_json_string(buffer, "SecretAccessKey", secret_key, sizeof(secret_key)) == 0) {
        printf("Secret Access Key: %s\n", secret_key);
    } else {
        fprintf(stderr, "Error: Could not extract SecretAccessKey from response\n");
    }
    
    // Extract the session token
    if (extract_json_string(buffer, "Token", session_token, sizeof(session_token)) == 0) {
        // Print the first part of the token and indicate it's truncated
        printf("Session Token:     %.40s...(truncated)\n", session_token);
    } else {
        fprintf(stderr, "Error: Could not extract Token from response\n");
    }
    
    // Extract the expiration time
    if (extract_json_string(buffer, "Expiration", expiration, sizeof(expiration)) == 0) {
        printf("Expiration:        %s\n", expiration);
    } else {
        fprintf(stderr, "Error: Could not extract Expiration from response\n");
    }
    
    printf("\n=== Environment Variable Format ===\n\n");
    printf("export AWS_ACCESS_KEY_ID=\"%s\"\n", access_key);
    printf("export AWS_SECRET_ACCESS_KEY=\"%s\"\n", secret_key);
    printf("export AWS_SESSION_TOKEN=\"%s\"\n", session_token);
    
    printf("\n=== End of Credentials ===\n");
    
    return 0;
}