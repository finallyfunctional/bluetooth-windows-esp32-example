# Bluetooth Windows Esp32 Example

This repository is a sample on how to connect from a Windows 10 PC to an ESP32 via bluetooth and windows sockets. You should pair your PC with the ESP32 first. The console application will loop through the BT devices connected to the PC and find the ESP32, connect to it, send a message, and recieve a message. The Arduino.ino file should be loaded onto the ESP32. The Visual Studio project should be built (I used Visual Studio 2019). The actual source code is just contained in the .ino file and single .cpp file. The code should be simple to follow, unlike most of the examples and documentation I came across.

# Steps to build:
- Download the zip with the code (or clone it with git).
- Download Visual Studio Community (tested with 2019). https://visualstudio.microsoft.com/downloads/
- During that installation, you only need the .NET desktop development kit.
- Download the Arduino IDE. https://www.arduino.cc/en/software
- Open the Arduino.ino file, and click Upload with your ESP32 plugged in. (Make sure you installed [ESP32 libraries first](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/).

- Open the .sln file in WindowsBTWithEsp32 with Visual Studio Community. 
- Click play (Local Windows Debugger).
- You should now receive messages via bluetooth in the terminal that opened: "Message from ESP32".


## References

* Helpful examples on getting connected devices - https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4k.html
* Bluetooth APIs - https://docs.microsoft.com/en-us/windows/win32/api/bluetoothapis/
* Microsoft example - https://github.com/microsoftarchive/msdn-code-gallery-microsoft/blob/master/Official%20Windows%20Platform%20Sample/Bluetooth%20connection%20sample/%5BC%2B%2B%5D-Bluetooth%20connection%20sample/C%2B%2B/bthcxn.cpp
* Bluetooth programming with Windows sockets - https://docs.microsoft.com/en-us/windows/win32/bluetooth/bluetooth-programming-with-windows-sockets