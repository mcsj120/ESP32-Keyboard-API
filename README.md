Enables a ESP32 to operate as a keyboard by recieving instructions from an API on what characters to type. Includes built-in shortcuts for teams operations.



To run the applicaiton, you will need to create a variables.cpp to store secrets for connecting to the desired API.

Enables an ESP32 to operate as a keyboard by receiving instructions from an API on what characters to type. Includes built-in shortcuts for Teams operations.

## Functionality

The main functionality is implemented in [main.cpp](./src/main.cpp). The key features include:

- **API Communicatio**: Connects to a specified server and port to receive instructions on what characters to type using credentials stored in `variables.cpp`.
- **Keyboard Emulation**: Uses the `BleKeyboard` library to emulate a Bluetooth keyboard.
- **Built-in Shortcuts**: Includes built-in shortcuts for Microsoft Teams operations, such as navigating chats and sending messages.

## Setup

To run the application, you will need to create a `variables.cpp` file to store secrets for connecting to the desired API. The file should look like this:

```cpp
const char* ssid = "Your_SSID";
const char* password = "Your_Password";
const char* server = "Your_Server_IP";
int port = Your_Port_Number;
```

Following that, platform.io was utilized to upload the application to an ESP32 server and kick it off. It should run continously from there as long as the ESP32 has power.