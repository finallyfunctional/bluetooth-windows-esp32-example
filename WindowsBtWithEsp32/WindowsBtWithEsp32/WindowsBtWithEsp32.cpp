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

bool sendMessageToEsp32()
{
    const char* message = "Message from Windows\r\n";
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

bool recieveMessageFromEsp32()
{
    printf("Waiting to recieve a message\r\n");
    while (true)
    {
        char recievedMessage[512]; //make sure buffer is big enough for any messages your receiving
        int recievedMessageLength = 512;
        int recieveResult = recv(btClientSocket, recievedMessage, recievedMessageLength, 0); //if your socket is blocking, this will block until a
        if (recieveResult < 0)                                                               //a message is recieved. If not, it will return right 
        {                                                                                    //away
            continue;
        }
        printf("Message recieved - \r\n");
        for (int i = 0; i < recieveResult; i++)
        {
            printf("%c", recievedMessage[i]);
        }
        printf("\r\n");
        sendMessageToEsp32();
    }
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
    if (!sendMessageToEsp32()) //send a message to the ESP32
    {
        return 0;
    }
    recieveMessageFromEsp32(); //receive messages from ESP32
    return 0;
}