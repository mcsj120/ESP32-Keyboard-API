#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BleKeyboard.h>

#include <variables.h>

#include <chrono>
#include <ctime>

//https://gist.github.com/ekaitz-zarraga/2b25b94b711684ba4e969e5a5723969b
// BleKeyboard bleKeyboard("ESP32 KB - M", "Espressif", 100);
BleKeyboard bleKeyboard("BT Keyboard", "Espressif", 100);

extern const char* ssid;
extern const char* password;
extern const char* server;
extern int port;

enum OS { Windows, Mac};

OS current_os = Mac;

const uint8_t KEY_SPACE = 32;

const uint8_t get_gui_key(){
    if(current_os == Mac){
        return KEY_LEFT_GUI;
    } else {
        return KEY_LEFT_CTRL;
    }
}

//#TODO: Sync esp32 datetime with ntp server to get real time

void printDateTime(){
    /*
    auto currentTime = std::chrono::system_clock::now();

    // Convert the time point to a time_t object
    std::time_t time = std::chrono::system_clock::to_time_t(currentTime);

    // Convert the time_t object to a local time structure
    std::tm* localTime = std::localtime(&time);

    // Extract date and time components
    int year = localTime->tm_year + 1900; // Years since 1900
    int month = localTime->tm_mon + 1;    // Months are 0-11
    int day = localTime->tm_mday;         // Day of the month
    int hour = localTime->tm_hour;        // Hour (0-23)
    int minute = localTime->tm_min;       // Minute (0-59)
    int second = localTime->tm_sec;       // Second (0-59)

    // Print the current date and time
    Serial.printf("%d-%d-%d %d:%d:%d", year, month, day, hour, minute, second);
    */
}

//Template enables char ptrs and strings to be passed through
template <typename T>
void logger(bool newLine, T info) {
    time_t currentTime = time(NULL);
    // Format and print the current date and time
    printDateTime();

    size_t (Print::*functionPtr)(const String &);

    if(newLine){
        functionPtr = &HardwareSerial::println;
    } else {
        functionPtr = &HardwareSerial::print;
    }
    (Serial.*functionPtr)(info);
}

/**
 * @brief Prints the messages in chunks to prevent issues where to much info is sent to the computer at once
 * 
 * @param input The string you want to print on the computer;
 */
void print_chunks(String input){
    int chunkSize = 5;
    int length = input.length();
    size_t numChunks = (length + chunkSize - 1) / chunkSize;
     
    for (size_t i = 0; i < numChunks; i++) {
        size_t startPos = i * chunkSize;
        size_t chunkLength;
        if(chunkSize < length - startPos){
            chunkLength = chunkSize;
        } else {
            chunkLength = length - startPos;
        }
        bleKeyboard.print(input.substring(startPos, startPos + chunkLength));
        delay(200);
    }
}

void setup() {
    Serial.begin(115200);
    delay(10);
    logger(true, "");
    
    WiFi.begin(ssid, password);             // Connect to the network
    logger(false, "Connecting to ");
    logger(false, ssid);

    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
        delay(500);
        logger(false, "...");
    }

    logger(true, "Connection established!");  
    logger(false, "IP address:\t");
    logger(true, WiFi.localIP().toString());         // Send the IP address of the ESP32 to the computer

    bleKeyboard.begin();
}

void loop() {
    WiFiClient client;
    if (client.connect(server, 80)) {
        client.println("GET /keyboard HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println();

        int retries = 5;
        bool success = false;
        while(retries > 0 && !success){
            if(client.available()){
                success = true;
                String line = client.readStringUntil('\n');
                if (line.startsWith("HTTP/1.1")) {
                    int statusCode = line.substring(9, 12).toInt();
                    if(statusCode == 200){
                        bool endHeaders = false;
                        String content;
                        while(client.available()){
                            if(!endHeaders){
                                line = client.readStringUntil('\n');
                                if (line == "\r") {
                                    endHeaders = true;
                                }
                            } else {
                                content = client.readStringUntil('\n');
                                break;
                            }
                        }
                        if(!content.isEmpty() && bleKeyboard.isConnected()){
                            if(content == "setup1"){
                                bleKeyboard.print("z");
                            } else if(content == "setup2"){
                                bleKeyboard.write(KEY_NUM_SLASH);
                            } else if(content == "enter"){
                                bleKeyboard.write(KEY_RETURN);
                            } else if(content == "switch") {
                                if(current_os == Mac){
                                    current_os = Windows;
                                } else {
                                    current_os = Mac;
                                }
                            }else if(content == "chat"){
                                // Opens the chat section of teams
                                bleKeyboard.press(get_gui_key());
                                bleKeyboard.press(KEY_NUM_3);
                                delay(100);
                                bleKeyboard.releaseAll();
                            } else if(content == "teams"){
                                // Switches to teams if teams is currently not selected
                                bleKeyboard.press(get_gui_key());
                                bleKeyboard.press(KEY_SPACE);
                                delay(200);
                                bleKeyboard.releaseAll();
                                bleKeyboard.print("teams");
                                // Give mac a sec to load up teams
                                delay(700);
                                bleKeyboard.write(KEY_RETURN);
                            } else if(content == "del"){
                                // Press Ctrl + A, and then backspace it
                                bleKeyboard.press(get_gui_key());
                                bleKeyboard.press(0x04);
                                delay(100);
                                bleKeyboard.releaseAll();
                                bleKeyboard.write(KEY_BACKSPACE);
                            } else if(content == "up"){
                                // Goes to the chat above
                                bleKeyboard.press(KEY_LEFT_ALT);
                                bleKeyboard.press(KEY_UP_ARROW);
                                delay(100);
                                bleKeyboard.releaseAll();
                            } else if(content == "down"){
                                // Goes to the chat below
                                bleKeyboard.press(KEY_LEFT_ALT);
                                bleKeyboard.press(KEY_DOWN_ARROW);
                                delay(100);
                                bleKeyboard.releaseAll();
                            } else {
                                //TODO: Split content out into bits of 5 and delay 100ms
                                print_chunks(content);
                            }
                        } else {
                            logger(false, "Lost content: ");
                            logger(false, content);
                        }
                    }
                }
            } else {
                retries = retries - 1;
                delay(500);
            }
        }
        if(retries == 0){
            logger(true, "client was not availible?");
        }

        client.stop();
    } else {
        logger(true, "Unable to connect to the endpoint");
    }

    if(!bleKeyboard.isConnected()) {
        logger(true, "Keyboard is not connected");
    }
    delay(200);
}