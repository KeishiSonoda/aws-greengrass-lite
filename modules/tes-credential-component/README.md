# TES Credential Component

A simple AWS IoT Greengrass component that demonstrates how to use the Token Exchange Service (TES) to obtain AWS credentials.

## Overview

The TES Credential Component is a minimal example that shows how to:

1. Connect to the local Token Exchange Service (TES)
2. Request AWS credentials
3. Parse and display the credentials in a readable format

This component is designed for educational purposes to help Greengrass component developers understand how to use the TES service in their own components.

## Prerequisites

- AWS IoT Greengrass Core v2.0 or later
- Token Exchange Service (TES) configured and running
- Appropriate IAM roles configured for the Greengrass core device

## Installation

### Using the AWS IoT Greengrass CLI

1. Build the component:
   ```
   cd modules/tes-credential-component
   mkdir -p build
   cd build
   cmake ..
   make
   ```

2. Create a component artifact:
   ```
   mkdir -p artifacts
   cp bin/tes-credential-component artifacts/
   ```

3. Deploy the component using the Greengrass CLI:
   ```
   aws greengrassv2 create-component-version \
     --inline-recipe file://recipe/recipe.yaml \
     --artifacts artifacts=file://artifacts/
   ```

### Using the AWS Management Console

1. Build the component as described above
2. Package the component
3. Upload the component to an S3 bucket
4. Create a component in the AWS IoT Greengrass console using the recipe and artifact

## Usage

Once deployed, the component will run automatically and output the AWS credentials to the component's log. You can view the logs using the Greengrass CLI:

```
greengrass-cli logs get -c com.example.TesCredentialComponent
```

The output will include:
- AWS Access Key ID
- AWS Secret Access Key
- AWS Session Token (truncated for security)
- Expiration time
- Environment variable format for easy export

## How It Works

The component uses a Unix domain socket to communicate with the TES service:

1. Creates a socket connection to `/run/greengrass/aws_iot_tes`
2. Sends a JSON request: `{"method":"request_credentials_formatted","params":{}}`
3. Receives a JSON response containing the AWS credentials
4. Parses the response to extract credential information
5. Displays the credentials in a readable format

## Code Structure

- `tes-credential-component.c`: Main component code
- `recipe/recipe.yaml`: Component recipe defining metadata and dependencies
- `CMakeLists.txt`: Build configuration

## Customization

To customize this component for your own use:

1. Modify the request parameters in the `request` variable if needed
2. Adjust the credential display format to suit your requirements
3. Add additional error handling or retry logic for production use

## Security Considerations

- This component prints credentials to stdout, which is suitable for educational purposes but not for production use
- In a real-world component, you should handle credentials securely and avoid logging them
- The component truncates the session token when displaying it to avoid exposing the full token

## Troubleshooting

Common issues:

1. **Connection refused**: Ensure the TES service is running and the socket path is correct
2. **Permission denied**: Check that the component has the necessary permissions to access the TES socket
3. **Invalid credentials**: Verify that the Greengrass core device has the correct IAM role configuration

## License

This component is licensed under the Apache License 2.0.