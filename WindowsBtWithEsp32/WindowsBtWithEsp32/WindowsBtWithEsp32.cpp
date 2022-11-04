/**
Refererences:
Helpful examples on getting connected devices - https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4k.html
Bluetooth APIs - https://docs.microsoft.com/en-us/windows/win32/api/bluetoothapis/
Microsoft example - https://github.com/microsoftarchive/msdn-code-gallery-microsoft/blob/master/Official%20Windows%20Platform%20Sample/Bluetooth%20connection%20sample/%5BC%2B%2B%5D-Bluetooth%20connection%20sample/C%2B%2B/bthcxn.cpp
Bluetooth programming with Windows sockets - https://docs.microsoft.com/en-us/windows/win32/bluetooth/bluetooth-programming-with-windows-sockets
**/

#pragma comment(lib, "Bthprops.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <stdlib.h>
#include <stdio.h>
#include <Winsock2.h>
#include <Ws2bth.h>
#include <BluetoothAPIs.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

BTH_ADDR esp32BtAddress;
SOCKADDR_BTH btSocketAddress;
SOCKET btClientSocket;

/// <summary>
/// Gets the bluetooth devices paired with this machine and 
/// finds an ESP32. If it finds one, its BT address is stored.
/// </summary>
bool getPairedEsp32BtAddress()
{
    BLUETOOTH_DEVICE_SEARCH_PARAMS btDeviceSearchParameters =
    {
      sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS), //size of object
      1, //return authenticated devices
      0, //return remembered devices
      1, //return unknown devices
      1, //return connected devices
      1, //issue inquery
      2, //timeout multipler. Multiply this value by 1.28 seconds to get timeout.
      NULL //radio handler
    };
    BLUETOOTH_DEVICE_INFO btDeviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO),0 }; //default
    HBLUETOOTH_DEVICE_FIND btDevice = NULL;
    btDevice = BluetoothFindFirstDevice(&btDeviceSearchParameters, &btDeviceInfo); //returns first BT device connected to this machine
    if (btDevice == NULL)
    {
        printf("Could not find any bluetooth devices.\r\n");
        return false;
    }
    do
    {
        wprintf(L"Checking %s.\r\n", btDeviceInfo.szName);
        if (wcsncmp(btDeviceInfo.szName, L"ESP32", 5) == 0)
        {
            printf("ESP32 found!\r\n");
            if (btDeviceInfo.fAuthenticated)  //I found that if fAuthenticated is true it means the device is paired.
            {
                printf("ESP32 is authenticated.\r\n");
                esp32BtAddress = btDeviceInfo.Address.ullLong;
                return true;
            }
            else
            {
                printf("This ESP32 is not authenticated. Please pair with it first.\r\n");
            }
        }
    } while (BluetoothFindNextDevice(btDevice, &btDeviceInfo)); //loop through remaining BT devices connected to this machine

    printf("Could not find a paired ESP32.\r\n");
    return false;
}

/// <summary>
/// Windows sockets need an initialization method called before they are used.
/// </summary>
bool startupWindowsSocket()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    int wsaStartupError = WSAStartup(wVersionRequested, &wsaData); //call this before using BT windows socket.
    if (wsaStartupError != 0)
    {
        printf("WSAStartup failed with error: %d\n", wsaStartupError);
        return false;
    }
    return true;
}

/// <summary>
/// Sets up bluetooth socket to communicate with ESP32.
/// </summary>
bool connectToEsp32()
{
    btClientSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM); //initialize BT windows socket
    memset(&btSocketAddress, 0, sizeof(btSocketAddress));
    btSocketAddress.addressFamily = AF_BTH;
    btSocketAddress.serviceClassId = RFCOMM_PROTOCOL_UUID;
    btSocketAddress.port = 0; //port needs to be 0 if the remote device is a client. See references.
    btSocketAddress.btAddr = esp32BtAddress; //this is the BT address of the remote device.
    if (connect(btClientSocket, (SOCKADDR*)&btSocketAddress, sizeof(btSocketAddress)) != 0) //connect to the BT device.
    {
        printf("Could not connect socket to ESP32. Error %ld\r\n", WSAGetLastError());
        return false;
    }
    unsigned long nonBlockingMode = 1;
    if (ioctlsocket(btClientSocket, FIONBIO, (unsigned long*)&nonBlockingMode) != 0) //set the socket to be non-blocking, meaning
    {                                                                                //it will return right away when sending/recieving
        printf("Could not set socket to be non-blocking.\r\n");
        return false;
    }
    return true;
}

bool sendMessageToEsp32(const char* message)
{
    int sendResult = send(btClientSocket, message, (int)strlen(message), 0); //send your message to the BT device
    if (sendResult == SOCKET_ERROR)
    {
        wprintf(L"Sending to ESP32 failed. Error code %d\r\n", WSAGetLastError());
        closesocket(btClientSocket);
        WSACleanup();
        return false;
    }
    return true;
}

bool sendMessageToEsp32(std::string message)
{
    const char* msg = message.c_str();
    return sendMessageToEsp32(msg);
   
}


//receive a message and send a message back.
//to not send a message back use send_reply="-". (This will block the esp32 if it expects a message if you don't send anywhere else!)
//always terminate the reply with \n. \n will send the minimum info to not block the esp32.
std::string recieveStringMessageFromEsp32(std::string send_reply = "\n", bool debug = true)
{
        char* buffer = new char[50000];
        int nrReceivedBytes = recv(btClientSocket, buffer, 50000, 0); //if your socket is blocking, this will block until a message comes in
        if (nrReceivedBytes < 0) {
            delete[] buffer; //make sure to clean up things on the heap made with "new" 
            return ""; 
        }              
                          
        std::string message(buffer, nrReceivedBytes);

        if(debug)
            std::cout << "Message recieved: " <<message << std::endl;

        if (send_reply != "-")
        {
            sendMessageToEsp32(send_reply); // send message to unblock the ESP32 from waiting on a message from the PC.
        }
        delete[] buffer; //make sure to clean up things on the heap made with "new"
        return message;
}

//returns <nrOfMessages, nrOfSeconds> measured.
//you can give up an expected message to see if your data is validated. MAKE SURE TO MATCH THE WHITESPACE (so terminate with \r\n).
void benchmarkMessagesPerSecond(std::string expectedMessage ="-", float measureXseconds = 10.0f )
{
    clock_t start, time = 0;
    start = clock();
    
    int validMessagesCounter = 0;
    int invalidMessagesCounter = 0;

    std::cout << "Benchmark started, wait " << measureXseconds << " seconds" << std::endl;

    if (expectedMessage != "-")
    {
        while (time < measureXseconds)
        {
            std::string msg = recieveStringMessageFromEsp32("\n", false);

            if (msg == "")
            {
                // do nothing, no message was received in the time period
            }
            else if (msg == expectedMessage)
            {
                validMessagesCounter++;
            }
            else
            {
                invalidMessagesCounter++;
            }
            time = (clock() - start) / CLOCKS_PER_SEC;
        }
    }
    else
    {
        while (time < measureXseconds)
        {
            std::string msg = recieveStringMessageFromEsp32("\n", false);
            if (msg == "")
            {
                // do nothing, no message was received in the time period
            }
            else
            {
                validMessagesCounter++;
            }
            time = (clock() - start) / CLOCKS_PER_SEC;
        }
    }
    std::cout << "Benchmark completed" << std::endl;
    std::cout << "Measured for " << time << " seconds" << std::endl;
    std::cout << "Received " << validMessagesCounter << " valid messages and " << invalidMessagesCounter << " invalid messages" << std::endl;
    std::cout << "Total: " << validMessagesCounter / time << " messages per second." << std::endl;
}



int main()
{
    printf("Attempting to find paired ESP32 devices.\r\n");
    if (!getPairedEsp32BtAddress()) //find an ESP32 paired with this machine
    {
        return 0;
    }
    if (!startupWindowsSocket()) //initialize windows sockets
    {
        return 0;
    }
    if (!connectToEsp32()) //initialize BT windows socket for connecting to ESP32
    {
        return 0;
    }
    if (!sendMessageToEsp32("\n")) //send a message to the ESP32
    {
        return 0;
    }
    std::cout <<"Waiting to recieve a message\r\n";
    //while (true)
    //{
    //    recieveStringMessageFromEsp32(); //receive messages from ESP32
    //}
    benchmarkMessagesPerSecond("<970,400,500-500,500,100-1,0>\r\n", 10);

    return 0;
}